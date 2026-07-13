# coding:utf-8
import codecs
import datetime
import os
import sys
import shutil
import re
import logging
import signal

try:
    if sys.version_info[0] == 3:
        import configparser
    elif sys.version_info[0] == 2:
        import ConfigParser as configparser
except ImportError:
    # python 2.6版本sys.version_info是一个元组
    import ConfigParser as configparser

from utils import (change_log_config, BACKUP_LOG_FILE, LEVEL, execute_cmd,
                   calculate_free_space_by_df, calculate_used_space_by_du,
                   KS_SSR_PID, get_file_content, proc_lock, mkdir_p)
from rollback import RollBack, setup_logger as setup_logger_r

logger = logging.getLogger(__name__)
current_log = BACKUP_LOG_FILE.format(datetime=datetime.datetime.strftime(datetime.datetime.now(), "%Y-%m%d-%H%M"))


def setup_logger(log_file=None, name='KSSSRLogger_BACKUP', console=True):
    global logger
    logger = change_log_config(LEVEL, console, log_file if log_file else current_log, name)


class Backup(object):

    def __init__(self, conf_path):
        self._conf_path = conf_path
        self.backup_dir_list = []
        self.exclude_dir_list = []
        self.store_path = None
        self.backup_exclude_map = {}
        # 记录系统上安装的rpm包文件列表(排除掉已经备份的文件)
        self.backup_rpm_file_path = '/opt/ks-ssr/rpm_backup_file.txt'
        # 用于剩余空间计算时记录系统上安装的rpm包文件列表
        self.backup_check_rpm_file_path = '/opt/ks-ssr/rpm_backup_check_file.txt'
        self.back_done_flag = None
        self.back_done_list = []
        # 存放subprocess调用的进程id的集合
        self.pid_list = set()
        # 备份进度记录文件
        self.progress_file = '/tmp/ks-ssr-backup-progress'
        if not os.path.exists(os.path.dirname(self.backup_rpm_file_path)):
            os.makedirs(os.path.dirname(self.backup_rpm_file_path))

        # 初始化进度文件
        with open(self.progress_file, "w") as fd:
            fd.write("0")

    def _get_default_conf_section(self, section_name):
        if self.is_path_exists(self._conf_path)[0] is False:
            return False, None

        cfg_parser = configparser.ConfigParser()
        if not cfg_parser.read(self._conf_path):
            return False, None

        options_list = cfg_parser.options(section_name)

        return True, dict(
            zip(
                options_list,
                [cfg_parser.get(section_name, opt) for opt in options_list],
            )
        )

    def write_progress(self, phase):
        self.back_done_list.append(phase)
        all_phases = ['rpm_file_list', 'generate_boot']
        all_phases.extend(self.backup_dir_list)
        current_progress = int(float(len(self.back_done_list)) / len(all_phases) * 100)
        with open(self.progress_file, "a+") as fd:
            fd.write("%d\n" % current_progress)

    @staticmethod
    def clean(path):
        if path is not None and os.path.exists(path):
            shutil.rmtree(path)

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

    def generate_and_check_backup_path(self, section_name='backup'):
        try:
            status, backup_conf_dict = self._get_default_conf_section(section_name)
            if status is False:
                return False, 1, 'Read %s Failed' % self._conf_path

            backup_dir = backup_conf_dict.get('backup_dir')
            exclude_dir = backup_conf_dict.get('exclude_dir')
            store_path = backup_conf_dict.get('store_path')
            if not all([backup_dir, store_path]):
                return False, 2, 'backup_dir:%s or store_path:%s is not exists' % (backup_dir, store_path)

            backup_dir_list = backup_dir.split(',')
            exclude_dir_list = []
            if exclude_dir:
                exclude_dir_list = exclude_dir.split(',')

            for back_path in backup_dir_list:
                back_path = str(back_path).strip().rstrip('/')
                if not back_path or not os.path.exists(back_path):
                    continue

                if back_path not in self.backup_dir_list:
                    self.backup_dir_list.append(back_path)

            # 排除ks-ssr-timeshift运行标识、系统日志
            exclude_dir_list.extend([KS_SSR_PID, self.backup_rpm_file_path, self.backup_check_rpm_file_path])
            self.exclude_dir_list = list(set([str(_).strip().rstrip('/') for _ in exclude_dir_list if _]))
            self.store_path = str(store_path) if not self.store_path else self.store_path

            self.back_done_flag = os.path.join(self.store_path, ".back_done_flag")

        except Exception as e:
            return False, 5, e

        return True, 0, None

    def generate_backup_exclude_map(self):
        for backup_path in self.backup_dir_list:
            exclude_paths = self.backup_exclude_map.setdefault(backup_path, [])
            for exclude_path in self.exclude_dir_list:
                if exclude_path.startswith(backup_path):
                    exclude_paths.append(exclude_path)

    def execute_backup(self):
        try:
            # 备份文件
            for bak_path in self.backup_dir_list:
                exclude_paths = self.backup_exclude_map.get(bak_path, [])
                bak_path = bak_path.rstrip('/')
                exclude_cmd = ''
                if exclude_paths:
                    for exclude_path in exclude_paths:
                        # exclude必须为相对路径（备份的文件夹最后一段开始计算）
                        _exclude_path = bak_path.split('/')[-1] + '/' + \
                                        re.sub(r'%s' % bak_path, '', exclude_path, count=1).lstrip('/')
                        exclude_cmd += ' --exclude ' + _exclude_path
                if exclude_cmd:
                    _cmd = 'rsync -av %s %s %s' % (bak_path, exclude_cmd, self.store_path)
                else:
                    _cmd = 'rsync -av %s %s' % (bak_path, self.store_path)
                status, err_code, rst = execute_cmd(_cmd, logger, pid_list=self.pid_list)
                self.write_progress(bak_path)
                if status is not True:
                    logger.error("backup dir %s failed!" % bak_path)
                    return False, 1, 'Failed to execute backup: %s ' % _cmd

            # 生成 rpm file list
            generate_rpm_file_cmd = "for each in $(rpm -qa|xargs rpm -ql); do if [ -f $each ] || [ -L $each ]; " \
                                    "then echo ""; if [ ! -e %s$each ]; then echo $each >> %s; fi; fi; done" \
                                    % (self.store_path, self.backup_rpm_file_path)
            execute_cmd(generate_rpm_file_cmd, logger)

            # 备份rpm list
            backup_rpm_cmd = "rsync -av --ignore-missing-args --ignore-existing --files-from=%s / %s" \
                             % (self.backup_rpm_file_path, self.store_path)
            status1, err_code1, rst1 = execute_cmd(backup_rpm_cmd, logger, pid_list=self.pid_list)
            self.write_progress('rpm_file_list')
            if status1 is not True:
                logger.info("Start backup files leaved over.")
                _backup_rpm_cmd = "rsync -av --ignore-existing --files-from=%s / %s" \
                                  % (self.backup_rpm_file_path, self.store_path)
                _status1, _err_code1, _rst1 = execute_cmd(_backup_rpm_cmd, logger, pid_list=self.pid_list)
                if _status1 is not True:
                    return False, 2, 'Failed to execute backup: %s ' % _backup_rpm_cmd

        except Exception as e:
            return False, 3, 'Failed to execute backup: %s' % e

        return True, 0, None

    # sonarqube block off
    def device_space_check(self):
        if not os.path.exists(self.store_path):
            mkdir_p(self.store_path)

        status, err_code, rst = calculate_free_space_by_df(self.store_path, logger)
        if status is not True:
            return False, 1, rst, None

        try:
            free_dist = int(rst)
        except Exception as e:
            return False, 2, 'Failed to calculate the disk free size: %s' % e, None

        if os.path.exists(self.store_path):
            status, err_code, rst = calculate_used_space_by_du(self.store_path, logger)
            if status is not True:
                return False, 3, rst, None

            try:
                store_dist_spend = int(rst)
                free_dist += store_dist_spend
                logger.info("Compute store dist spend space(%s). \nCompute total backup free space(%s)"
                            % (store_dist_spend, free_dist))
            except Exception as e:
                return False, 4, 'Failed to calculate the store path size(%s)' % e, None

        need_dist_usage = 0
        try:
            # 计算指定备份路径的磁盘大小
            for bak_path in self.backup_dir_list:
                exclude_paths = self.backup_exclude_map.get(bak_path, [])
                status, err_code, rst = calculate_used_space_by_du(bak_path, logger)
                if status is not True:
                    return False, 5, rst, None

                total_exclude_size = 0
                for exclude_path in exclude_paths:
                    # 可能排除路径还没有产生，不存在的先跳过
                    if not os.path.exists(exclude_path):
                        continue

                    status, err_code, exclude_rst = calculate_used_space_by_du(exclude_path, logger)
                    if status is not True:
                        return False, 6, exclude_rst, None

                    total_exclude_size += int(exclude_rst)
                total_spend_space = int(rst) - total_exclude_size
                need_dist_usage += total_spend_space

                msg = 'Compute backup spend space(backup_path: %s spend_dist: %s; ' \
                      'exclude_paths: %s exclude_dist: %s ) total_spend %s' \
                      % (bak_path, int(rst), exclude_paths, total_exclude_size, total_spend_space)
                logger.info(msg)

            # 计算备份的rpm包库文件列表大小
            execute_cmd('rm -f %s' % self.backup_check_rpm_file_path, logger)
            generate_rpm_file_cmd = "for each in $(rpm -qa|xargs rpm -ql); do if [ -f $each ] || [ -L $each ]; " \
                                    "then echo $each >>%s;fi; done;" % self.backup_check_rpm_file_path
            execute_cmd(generate_rpm_file_cmd, logger)
            rpm_file_dist_spend = 0
            with open(self.backup_check_rpm_file_path, 'r') as f:
                for line in f:
                    line = line.strip('\n')
                    exists_backup_dirs = False
                    exists_backup_path = None
                    for bak_path in self.backup_exclude_map.keys():
                        if line.startswith(bak_path):
                            exists_backup_dirs = True
                            exists_backup_path = bak_path
                            break

                    need_rpm_backup = False
                    if exists_backup_dirs is True:
                        exists_exclude_dirs = False
                        for exclude_path in self.backup_exclude_map[exists_backup_path]:
                            if line.startswith(exclude_path):
                                exists_exclude_dirs = True
                                break
                        if exists_exclude_dirs is True:
                            need_rpm_backup = True
                    else:
                        need_rpm_backup = True

                    if need_rpm_backup:
                        status, err_code, spend_space_rst = calculate_used_space_by_du(line, logger, is_check_rpm_file=True)
                        if status is not True:
                            return False, 7, spend_space_rst, None

                        rpm_file_dist_spend += int(spend_space_rst)
            logger.info("Compute backup rpm file dist spend(%s)" % rpm_file_dist_spend)
            need_dist_usage += rpm_file_dist_spend
            logger.info('Compute backup total spend space: %s' % need_dist_usage)
        except Exception as e:
            return False, 8, 'Failed to calculate the disk usage size(%s)' % e, {}

        if free_dist < need_dist_usage:
            msg = 'Disk space is not enough for backup; free: %s spend: %s' % (free_dist, need_dist_usage)
            return False, 9, msg, {'backup_path_free_space': free_dist, 'backup_spend_space': need_dist_usage}

        return True, 0, '', {'backup_path_free_space': free_dist, 'backup_spend_space': need_dist_usage}
    # sonarqube block on

    @staticmethod
    def boot_path_check():
        if os.path.exists('/boot'):
            status, err_code, rst = calculate_free_space_by_df('/boot', logger)
            if status is not True:
                return False, 1, rst

            free_space = round(int(rst) / 1024.0)
            if free_space <= 150:
                return False, 2, '/boot space is not enough(150M) for backup'
            elif 150 < free_space < 200:
                logger.warning('/boot space is only %sM remaining' % free_space)

        return True, 0, ''

    # sonarqube block off
    def check_readonly_mounts(self):
        gvfs_path = "/run/user/0/gvfs/"
        mounts = get_file_content("/proc/mounts", as_list=True)
        need_umount_list = []
        for line in mounts:
            _, mount_point, _, flags, _, _ = line.split()
            flags = flags.split(",")
            # 如果备份目录下存在/run目录，且/run/media存在相关的挂载，需要报错，避免后续备份空间计算
            if "/run/media" in mount_point:
                # 解决“/run/media/yue/CentOS 7 x86_64”读取出为"/run/media/yue/CentOS\\0407\\040x86_64"的显示问题
                mount_point = codecs.escape_decode(mount_point)[0].decode()

            if "ro" in flags:
                if mount_point == '/mnt' or mount_point == '/sys':
                    continue

                for back_path in self.backup_dir_list:
                    # 只读的挂载在备份目录下，不在排除目录中(回滚时写不进)
                    if mount_point.startswith(back_path) and mount_point not in self.exclude_dir_list:
                        need_umount_list.append(mount_point)

        if os.path.exists(gvfs_path) and len(os.listdir(gvfs_path)) > 0 \
                and gvfs_path not in need_umount_list:
            # 检查是否存在如ftp挂载等情况
            need_umount_list.append(gvfs_path)

        if need_umount_list:
            msg = "Stopping conversion due to  mount points below %s.\n" \
                  "Please umount these mount points before conversion." % ','.join(need_umount_list)
            return False, 1, msg, need_umount_list

        logger.info("The backup path did not detected a Read-only mount points.")
        return True, 0, '', need_umount_list
    # sonarqube block on

    def check_backup_path(self):
        if os.path.exists(self.store_path):
            if not os.path.exists(self.back_done_flag):
                return False, 1, 'Backup path exists! Please remove existed backup dir: %s.' % self.store_path
            else:
                logger.info("A directory with completed backup already exists and will be deleted.")
                shutil.rmtree(self.store_path)

        return True, 0, ''

    def backup_check(self):
        logger.info("Start check backup path whether or not exists")
        status, err_code, msg = self.check_backup_path()
        if status is not True:
            return status, err_code, msg

        logger.info("Start check readonly mounts")
        status, err_code, msg, need_umount_list = self.check_readonly_mounts()
        if status is not True:
            return status, err_code, msg
        logger.info("Start check boot size")
        status, err_code, msg = self.boot_path_check()
        if status is not True:
            return status, err_code, msg
        logger.info("Start check backup space")
        status, err_code, msg, _detail = self.device_space_check()
        if status is not True:
            return status, err_code, msg
        return True, 0, ''

    def signal_handler(self, signal_code, frame):
        logger.error('Received kill signal: %s' % signal_code)
        for pid in self.pid_list:
            os.killpg(pid, signal.SIGTERM)

        sys.exit(1)

    def run(self):
        logger.info("Start the backup process...")
        critical_msg = "err_code[{code}], err_detail[{detail}]"
        try:
            # 生成备份路径并校验
            logger.info("Start generate and check backup path")
            status, err_code, err_msg = self.generate_and_check_backup_path()
            if status is False:
                logger.critical(critical_msg.format(code=err_code, detail=err_msg))
                sys.exit(1)

            self.generate_backup_exclude_map()
            # 检查磁盘空间
            logger.info("Start backup check")
            status1, err_code1, err_msg1 = self.backup_check()
            if status1 is False:
                logger.critical(critical_msg.format(code=err_code1, detail=err_msg1))
                sys.exit(1)

            # 执行备份(已经执行过的，删除备份路径文件)
            logger.info("Start backup system files.")
            status2, err_code2, err_msg2 = self.execute_backup()
            if status2 is False:
                self.clean(self.store_path)
                logger.critical(critical_msg.format(code=err_code2, detail=err_msg2))
                sys.exit(1)

        except (Exception, SystemExit, KeyboardInterrupt) as e:
            logger.error("The backup process failed, error: %s" % (str(e)))
            sys.exit(1)

        # 创建完成标识
        os.mknod(self.back_done_flag)


def main(config):
    if not proc_lock():
        logger.warning("There is a ks-ssr-timeshift process running,exit!")
        exit(1)

    setup_logger_r(current_log, 'KSSSRLogger_BACKUP')

    ssr_backup = Backup(conf_path=config)
    # 开始监听SIGTERM信号
    signal.signal(signal.SIGTERM, ssr_backup.signal_handler)
    ssr_backup.run()

    # 执行生成rollback内核的动作
    ssr_rollback = RollBack(conf_path=config)
    ssr_rollback.run(gen_grub=False)
    ssr_backup.write_progress('generate_boot')
    logger.info("The backup process has ended")
