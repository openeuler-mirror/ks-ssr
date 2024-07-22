# coding:utf-8

import logging
import subprocess
import logging.config
import os.path
import errno
import fcntl

# 默认的一些日志配置
LEVEL = logging.DEBUG
LOG_BASE_DIR = "/var/log/kylinsec/ks-ssr/"
CHECK_LOG_FILE = os.path.join(LOG_BASE_DIR, "ks-ssr-check.log-{datetime}")
BACKUP_LOG_FILE = os.path.join(LOG_BASE_DIR, "ks-ssr-backup.log-{datetime}")
ROLLBACK_LOG_FILE = os.path.join(LOG_BASE_DIR, "ks-ssr-rollback.log-{datetime}")
TMP_BACKUP_LOG = "/tmp/ks-ssr-backup"
CONFIG_PATH = "/usr/share/ks-ssr/ssr.ini"

g_lock_fd = -1
KS_SSR_PID = "/run/lock/ks-ssr.lock"
LOCKDIR = "/run/lock/"

BASE_DIR_64 = "/usr/lib64/ks-ssr/timeshift"

CHECK_RESULT = {
    "result": "FAILED",  # SUCCESS FAILED WARNING
    "msg": "",
    "boot": 0.0,  # /boot分区剩余空间(MB)
    "backup_file_size": 0.0,  # 备份文件所需占用的空间(MB)
    "remaining_space_size": 0.0  # 备份目录所在分区的剩余空间(MB)
}


def get_logging_config(level, console, log_file):
    """
    生成logging配置信息
    :return:
    """
    return {
        'version': 1,
        'loggers': {
            'KSSSRLogger': {
                'handlers': ['file1', 'console'] if console else ['file1'],
                'level': level,
            },
            'KSSSRLogger_BACKUP': {
                'handlers': ['file1', 'file2', 'console'] if console else ['file1', 'file2'],
                'level': level,
            },
        },
        'handlers': {
            'console': {
                'formatter': 'fmt',
                'class': 'logging.StreamHandler',
                'level': 'INFO'
            },
            'file1': {
                'formatter': 'fmt',
                'class': 'logging.FileHandler',
                'level': 'DEBUG',
                'filename': log_file,
            },
            'file2': {
                'formatter': 'fmt',
                'class': 'logging.FileHandler',
                'level': 'DEBUG',
                'filename': TMP_BACKUP_LOG,
                'mode': "w"
            }
        },
        'formatters': {
            'fmt': {
                'format': '%(asctime)s [%(levelname)s] [%(thread)x] [%(filename)s:%(lineno)d] - %(funcName)s() - %(message)s',
                'datefmt': '%Y-%m-%d %H:%M:%S'
            }
        },
        'disable_existing_loggers': True,
    }


def change_log_config(level, console, log_file, name='KSSSRLogger'):
    """根据新的配置信息，重新加载日志配置"""
    logging.config.dictConfig(get_logging_config(level, console, log_file))
    return logging.getLogger(name)


def execute_cmd(cmd, logger, print_log=True):
    try:
        if print_log:
            logger.debug('run_cmd: %s' % cmd)
        rst = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE).communicate()
        if not rst[0]:
            return False, 1, None

        result = rst[0].decode("utf-8").strip('\n')
    except Exception as e:
        return False, 2, e

    return True, 0, result


def calculate_free_space_by_df(path, logger):
    if not os.path.exists(path):
        return False, 1, 'Failed to calculate the %s free size, path is not exists' % path

    try:
        free_dist_cmd = "df %s | awk 'NR==2{print $4}' | sed 's/G//g'" % path
        status, err_code, free_space = execute_cmd(free_dist_cmd, logger)
        if status is not True:
            return False, 2, 'Failed to calculate the %s free size' % path

        if not free_space:
            # 兼容centerOS6
            free_dist_cmd = "df %s | awk 'NR==3{print $3}' | sed 's/G//g'" % path
            status, err_code, free_space = execute_cmd(free_dist_cmd, logger)
            if status is not True:
                return False, 3, 'Failed to calculate the %s free size' % path

        free_space = int(free_space)
    except Exception as e:
        logger.warning(e)
        return False, 4, 'Failed to calculate the %s free size' % path

    logger.info("Compute %s free space(%s)" % (path, free_space))
    return True, 0, free_space


def calculate_total_space_by_df(path, logger):
    if not os.path.exists(path):
        return False, 1, 'Failed to calculate the %s total size, path is not exists' % path

    try:
        total_dist_cmd = "df %s | awk 'NR==2{print $2}' | sed 's/G//g'" % path
        status, err_code, total_space = execute_cmd(total_dist_cmd, logger)
        if status is not True:
            return False, 2, 'Failed to calculate the %s total size' % path

        if not total_space:
            # 兼容centerOS6
            total_dist_cmd = "df %s | awk 'NR==3{print $1}' | sed 's/G//g'" % path
            status, err_code, total_space = execute_cmd(total_dist_cmd, logger)
            if status is not True:
                return False, 3, 'Failed to calculate the %s total size' % path

        total_space = int(total_space)
    except Exception as e:
        logger.warning(e)
        return False, 4, 'Failed to calculate the %s total size' % path

    logger.info("Compute %s total size(%s)" % (path, total_space))
    return True, 0, total_space


def calculate_partition_by_df(path, logger):
    if not os.path.exists(path):
        return False, 1, 'Failed to calculate the %s partition info, path is not exists' % path

    partition_cmd = "df %s | awk 'NR==2{print $6}' | sed 's/G//g'" % path
    status, err_code, partition_info = execute_cmd(partition_cmd, logger)
    if status is not True:
        return False, 2, 'Failed to calculate the %s partition info' % path

    if not partition_info:
        # 兼容centerOS6
        partition_cmd = "df %s | awk 'NR==3{print $5}' | sed 's/G//g'" % path
        status, err_code, partition_info = execute_cmd(partition_cmd, logger)
        if status is not True:
            return False, 3, 'Failed to calculate the %s partition info' % path

    logger.info("Compute %s partition info(%s)" % (path, partition_info))
    return True, 0, partition_info


def calculate_used_space_by_du(path, logger):
    if not os.path.exists(path):
        return False, 1, 'Failed to calculate the %s used size, path is not exists' % path

    try:
        used_dist_cmd = "du -s %s | awk 'NR==1{print $1}'" % path
        status, err_code, used_space = execute_cmd(used_dist_cmd, logger)
        if status is not True:
            return False, 2, 'Failed to calculate the %s used size' % path

        used_space = int(used_space)
    except Exception as e:
        return False, 3, 'Failed to calculate the %s used size' % path

    return True, 0, used_space


def get_file_content(filename, as_list=False):
    """Return content of a file either as a list of lines or as a multiline
    string.
    """
    lines = []
    if not os.path.exists(filename):
        if not as_list:
            return ""
        return lines
    file_to_read = open(filename, "r")
    try:
        lines = file_to_read.readlines()
    finally:
        file_to_read.close()
    if as_list:
        # remove newline character from each line
        return [x.strip() for x in lines]

    return "".join(lines)


def mkdir_p(path):
    """Create all missing directories for the path and raise no exception
    if the path exists.
    """
    try:
        os.makedirs(path)
    except OSError as err:
        if err.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise


def proc_lock():
    if not os.path.exists(LOCKDIR):
        mkdir_p(LOCKDIR)
    global g_lock_fd
    try:
        g_lock_fd = open(KS_SSR_PID, 'w+')
        fcntl.flock(g_lock_fd, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except IOError:
        return None
    else:
        return True


def runcmd(command):
    ret = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
    stdout, stderr = ret.communicate()
    return (
        stdout.decode('utf-8').strip("\n"), stderr.decode('utf-8')
    )
