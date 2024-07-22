# coding:utf-8
import logging
import datetime
import sys
import json
import os
import shutil
import signal
from backup import Backup, setup_logger as setup_logger_b
from utils import change_log_config, CHECK_LOG_FILE, LEVEL, calculate_free_space_by_df, proc_lock, CHECK_RESULT

logger = logging.getLogger(__name__)
current_log = CHECK_LOG_FILE.format(datetime=datetime.datetime.strftime(datetime.datetime.now(), "%Y-%m%d-%H%M"))


def setup_logger(log_file=None, name='KSSSRLogger', console=False):
    global logger
    logger = change_log_config(LEVEL, console, log_file if log_file else current_log, name)


class Check(Backup):
    def __init__(self, conf_path, check_result):
        super(Check, self).__init__(conf_path)
        self.result = check_result
        self.check_done_flag = None
        self.is_terminal = False

    @staticmethod
    def boot_path_check():
        free_space = 0
        if os.path.exists('/boot'):
            status, err_code, rst = calculate_free_space_by_df('/boot', logger)
            if status is not True:
                return False, 1, rst, free_space

            free_space = round(int(rst) / 1024.0)
            if free_space <= 150:
                return False, 2, '/boot space is not enough(150M) for backup', free_space
            elif 150 < free_space < 200:
                logger.warning('/boot space is only %sM remaining' % free_space)

        return True, 0, '', free_space

    def check_backup_path(self):
        if os.path.exists(self.store_path):
            if not os.path.exists(self.check_done_flag):
                return False, 1, 'Backup path exists! Please remove existed backup dir: %s.' % self.store_path
        else:
            os.makedirs(self.store_path)
            os.mknod(self.check_done_flag)

        return True, 0, ''

    def backup_check(self):
        error_msg = ""
        logger.info("Start check backup path whether or not exists")
        status, err_code, msg = self.check_backup_path()
        if status is not True:
            error_msg += msg + "\n"

        logger.info("Start check readonly mounts")
        status, err_code, msg, need_umount_list = self.check_readonly_mounts()
        if status is not True:
            error_msg += msg + "\n"

        logger.info("Start check boot size")
        status, err_code, msg, boot_free_spec = self.boot_path_check()
        if status is not True:
            error_msg += msg + "\n"

        logger.info("Start check backup space")
        status, err_code, msg, backup_space_detail = self.device_space_check()
        if status is not True:
            error_msg += msg + "\n"
        return error_msg, boot_free_spec, backup_space_detail

    def signal_handler(self, signal_code, frame):
        self.is_terminal = True
        logger.error('Received kill signal: %s' % signal_code)
        self.result['msg'] = 'Received kill signal: %s' % signal_code
        if self.store_path and self.check_done_flag:
            if os.path.exists(self.store_path) and os.path.exists(self.check_done_flag):
                logger.info("Delete the directory created during the check phase: %s" % self.store_path)
                shutil.rmtree(self.store_path)
        sys.exit(1)

    def print_result(self):
        print(json.dumps(self.result))
        if self.is_terminal:
            sys.exit(15)

    # sonarqube block off
    def run(self):
        logger.info("Start the check process...")
        try:
            # 生成备份路径并校验
            logger.info("Start generate and check backup path")
            status, err_code, err_msg = self.generate_and_check_backup_path()
            if status is False:
                self.result['result'] = 'FAILED'
                self.result['msg'] = err_msg
                logger.critical("err_code[%s], err_detail[%s]" % (err_code, err_msg))
                sys.exit(1)

            self.generate_backup_exclude_map()
            self.check_done_flag = os.path.join(self.store_path, ".check_done_flag")
            # 检查磁盘空间
            logger.info("Start backup check")
            error_msg, boot_free_spec, backup_space_detail = self.backup_check()
            if error_msg:
                self.result['result'] = 'FAILED'
                self.result['msg'] = error_msg.strip() if error_msg else error_msg
            self.result['boot'] = boot_free_spec
            if not error_msg:
                if 150 <= boot_free_spec < 200:
                    self.result['result'] = "WARNING"
                    self.result['msg'] = '/boot space is only %sM remaining' % boot_free_spec
                else:
                    self.result['result'] = 'SUCCESS'

            if backup_space_detail:
                self.result['backup_file_size'] = round(int(backup_space_detail['backup_spend_space']) / 1024.0)
                self.result['remaining_space_size'] = round(int(backup_space_detail['backup_path_free_space']) / 1024.0)

            if error_msg:
                logger.critical(error_msg.strip())
                exit(1)

        except (Exception, SystemExit, KeyboardInterrupt) as e:
            logger.error("The backup process failed, error: %s" % (str(e)))
        finally:
            if self.store_path and self.check_done_flag:
                if os.path.exists(self.store_path) and os.path.exists(self.check_done_flag):
                    logger.info("Delete the directory created during the check phase: %s" % self.store_path)
                    shutil.rmtree(self.store_path)

        if self.result['result'] in ['SUCCESS', "WARNING"]:
            logger.info("The backup process successful!")
        logger.info("check result: %s" % json.dumps(self.result))
    # sonarqube block off


def main(config):
    if not proc_lock():
        msg = "There is a ks-ssr-timeshift process running,exit!"
        logger.warning(msg)
        CHECK_RESULT['msg'] = msg
        print(json.dumps(CHECK_RESULT))
        exit(1)
    setup_logger_b(current_log, 'KSSSRLogger', False)
    check_obj = Check(conf_path=config, check_result=CHECK_RESULT)
    # 开始监听SIGTERM信号
    signal.signal(signal.SIGTERM, check_obj.signal_handler)
    check_obj.run()
    check_obj.print_result()
