# coding:utf-8
import os
import shutil
import logging
import sys
import datetime

try:
    if sys.version_info.major == 3:
        import configparser
    elif sys.version_info.major == 2:
        import ConfigParser as configparser
except:
    # python 2.6版本sys.version_info是一个元组
    import ConfigParser as configparser

from utils import change_log_config, ROLLBACK_LOG_FILE, LEVEL, runcmd, proc_lock

logger = logging.getLogger(__name__)
current_log = ROLLBACK_LOG_FILE.format(datetime=datetime.datetime.strftime(datetime.datetime.now(), "%Y-%m%d-%H%M"))


def setup_logger(log_file=None, name='KSSSRLogger'):
    global logger
    logger = change_log_config(LEVEL, True, log_file if log_file else current_log, name)


class RollBack(object):
    def __init__(self, conf_path):
        self._conf_path = conf_path
        self.store_path = ""
        # 用于系统还原的vmlinuz
        self.vmlinuz = "/boot/vmlinuz-rollback"
        # 用于系统还原的initramfs
        self.initrd = "/boot/initramfs-rollback.img"
        # 存放rollback时所需脚本文件
        self.ssr_env_dir = "/usr/share/ks-ssr/timeshift"
        # 存放需打包进initrd中的脚本文件
        self.opt_ssr_dir = "/opt/ks-ssr/ks-ssr"
        # rollback模块是否打包进initrd的标志文件
        self.ssr_tag_dracut = "/opt/ks-ssr/.ks-ssr_dracut_flag"
        # initrd中是否集成服务文件的标志文件
        self.ssr_tag_service = "/opt/ks-ssr/.need_service"
        # centos6下base模块的init文件，centos6_init_scripts会对其进行修改
        self.centos6_dracut_init = "/usr/share/dracut/modules.d/99base/init"
        # centos6下特殊处理base模块
        self.centos6_init_scripts = "/usr/share/dracut/modules.d/99ksssr/centos6-init.sh"

        self._get_default_conf_section()
        self.backup_done_flag = os.path.join(self.store_path, ".back_done_flag")

    @staticmethod
    def is_path_exists(paths):
        if not paths:
            return False, paths

        if isinstance(paths, list):
            for _ in paths:
                if not os.path.exists(_):
                    return False, _

        elif isinstance(paths, str):
            if not os.path.exists(paths):
                return False, paths

        else:
            return False, paths

        return True, None

    def _get_default_conf_section(self, section_name='backup'):
        if self.is_path_exists(self._conf_path)[0] is False:
            return False, None

        cfg_parser = configparser.ConfigParser()
        if not cfg_parser.read(self._conf_path):
            return False, None

        self.store_path = cfg_parser.get(section_name, 'store_path')

    def _gen_flags(self):
        if os.path.exists(self.ssr_env_dir) and not os.path.exists(self.opt_ssr_dir):
            try:
                shutil.copytree(self.ssr_env_dir, self.opt_ssr_dir)
            except Exception as e:
                logger.info(e)

        if not os.path.exists(self.ssr_tag_dracut):
            os.mknod(self.ssr_tag_dracut)

        if not os.path.exists(self.ssr_tag_service):
            os.mknod(self.ssr_tag_service)

    def _check_backup_flag(self):
        if os.path.exists(self.backup_done_flag):
            logger.info("Backup done flag detected, OK!")
        else:
            logger.error("ERROR: Please finish system backup first before rollback!")
            if os.path.exists(self.ssr_tag_dracut):
                os.remove(self.ssr_tag_dracut)
            if os.path.exists(self.ssr_tag_service):
                os.remove(self.ssr_tag_service)
            exit(2)

    def _generate_vmlinuz(self):
        uname_r, err = runcmd("uname -r")
        logger.info("Generate rollback vmlinuz!")
        vmlinuz_org = "/boot/vmlinuz-" + uname_r

        if not os.path.exists(vmlinuz_org):
            cmd = "cat /proc/cmdline |awk '{print $1}'|awk -F '/' '{print $2}'"
            vmlinuz_ret, err = runcmd(cmd)
            vmlinuz_org = "/boot/" + vmlinuz_ret

        if os.path.exists(vmlinuz_org):
            try:
                shutil.copy(vmlinuz_org, self.vmlinuz)
            except Exception as e:
                logger.info(e)
        else:
            logger.critical("vmlinuz not found!!!")
            if os.path.exists(self.ssr_tag_dracut):
                os.remove(self.ssr_tag_dracut)
            if os.path.exists(self.ssr_tag_service):
                os.remove(self.ssr_tag_service)
            exit(1)
        logger.info("Generate rollback vmlinuz successful!")

    def _generate_initrd(self):
        uname_r, err = runcmd("uname -r")
        logger.info("Generate rollback initramfs!")
        if uname_r:
            cmd = "dracut -v -f -a 'lvm' %s %s" % (self.initrd, uname_r)
            runcmd(cmd)
        elif err:
            logger.warning(err)

    def _add_rollback_grub(self):
        if os.path.exists(self.vmlinuz) and os.path.exists(self.initrd):
            rm_cmd = "grubby --remove-kernel=%s" % self.vmlinuz
            runcmd(rm_cmd)
            cmd = 'grubby --add-kernel={vmlinuz} --initrd={initrd} --title="{title}" --copy-default --make-default ' \
                  '--args=rollback'.format(vmlinuz=self.vmlinuz, initrd=self.initrd, title="KylinSec Rollback")
            grbret, errgrb = runcmd(cmd)
            if errgrb:
                logger.warning(errgrb)
        else:
            logger.critical("vmlinuz or initramfs.img not found!")
            exit(1)
        logger.info("Add rollback grub entry successful!")

    def _centos6_rb_init(self):
        if os.path.exists(self.centos6_dracut_init) and os.path.exists(self.centos6_init_scripts):
            cmd = 'sh %s' % self.centos6_init_scripts
            runcmd(cmd)
            logger.info("Add init scripts for centos6!")

    def run(self, gen_grub=True):
        logger.info("Start generate rollback vmlinuz and initrd!")
        self._check_backup_flag()
        self._centos6_rb_init()
        self._gen_flags()

        if not os.path.exists(self.vmlinuz) or not os.path.exists(self.initrd):
            self._generate_vmlinuz()
            self._generate_initrd()
        else:
            logger.info("Apply existed vmlinz: %s, initramfs: %s" % (self.vmlinuz, self.initrd))

        # TODO 验证一下上面代码中的退出函数，程序退出后，标志文件是否会被删除
        if os.path.exists(self.ssr_tag_dracut):
            os.remove(self.ssr_tag_dracut)
        if os.path.exists(self.ssr_tag_service):
            os.remove(self.ssr_tag_service)
        if gen_grub:
            self._add_rollback_grub()
        logger.info("rollback vmlinuz and initrd get ready!")


def main(config):
    if not proc_lock():
        logger.warning("There is a ks-ssr-timeshift process running,exit!")
        exit(1)

    rbk_obj = RollBack(conf_path=config)
    rbk_obj.run()
    logger.info("Please reboot the system!")
