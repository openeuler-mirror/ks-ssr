# --coding:utf8 --

try:
    import configparser
except Exception:
    import ConfigParser as configparser

import os
import stat
import br.utils
import json
import br.vars
import re
import base64


if os.path.exists("/etc/logrotate.d/rsyslog"):
    LOGFILE_CONF_FILEPATH = "/etc/logrotate.d/rsyslog"
else:
    LOGFILE_CONF_FILEPATH = "/etc/logrotate.d/syslog"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH | stat.S_IXUSR

MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"
APPEND_ATTR_LIMIT = "append-permissions-limit"
RAW_DATA = "raw-data"
APPEND_ATTR_DATA = "append-attr-data"
ST_MODE = "st-mode"

FORMAT_STR = '{} "{}" {}'
GREP_CMD = "grep -r"

SYSLOG_PATHS = [
    "/var/log/cron",
    "/var/log/maillog",
    "/var/log/messages",
    "/var/log/secure",
    "/var/log/spooler",
]

# 轮转前，先去掉追加属性，否则会导致轮转失败
# 轮转后，加上追加属性
LOGFILE_ROTETE_CONF = "{0}\n\
{{\n\
    missingok\n\
    sharedscripts\n\
    prerotate\n\
        sudo /usr/bin/chattr -a {0}\n\
    endscript\n\
    postrotate\n\
        sudo /usr/bin/systemctl kill -s HUP rsyslog.service >/dev/null 2>&1 || true\n\
        sudo /usr/bin/chattr +a {0}\n\
    endscript\n\
}}\n"


# 系统日志文件和配置的权限控制
class Permissions:
    def __init__(self):
        pass

    def get(self):
        retdata = dict()

        # 日志权限是否设置
        has_mode_limit = True
        for log_file in SYSLOG_PATHS:
            if not os.access(log_file, os.F_OK):
                continue
            mode = os.stat(log_file).st_mode
            if (mode & EXCLUDE_MODE) != 0:
                has_mode_limit = False
                break
        retdata[MODE_PERMISSIONS_LIMIT] = has_mode_limit

        # 日志是否有追加属性
        has_attr_limit = True
        for log_file in SYSLOG_PATHS:
            if not os.access(log_file, os.F_OK):
                continue
            has_attr_limit = self.check_append_attr(log_file)
            if not has_attr_limit:
                break

        retdata[APPEND_ATTR_LIMIT] = has_attr_limit

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        # 设置权限
        self.set_permissions_limit(args[MODE_PERMISSIONS_LIMIT], SYSLOG_PATHS)
        # 设置追加属性
        self.set_attr_limit(args[APPEND_ATTR_LIMIT], SYSLOG_PATHS)

        # 轮转配置修改
        if args[APPEND_ATTR_LIMIT]:
            new_conf = ""  # 新配置
            for log_file in SYSLOG_PATHS:
                conf_has_file_path = br.utils.subprocess_has_output(
                    FORMAT_STR.format(GREP_CMD, log_file, LOGFILE_CONF_FILEPATH)
                )
                if len(conf_has_file_path) == 0:
                    continue
                new_conf += LOGFILE_ROTETE_CONF.format(log_file)

            # 写入配置
            if len(new_conf) > 0:
                br.utils.subprocess_not_output(
                    "truncate -s 0 {}".format(LOGFILE_CONF_FILEPATH)
                )
                br.utils.subprocess_not_output(
                    "echo '{}'    >> {}".format(new_conf, LOGFILE_CONF_FILEPATH)
                )
        else:
            # 在配置文件中删除含有chattr的行
            br.utils.subprocess_not_output(
                FORMAT_STR.format(
                    "sed -i '/chattr/d'", LOGFILE_CONF_FILEPATH, LOGFILE_CONF_FILEPATH
                )
            )

        return (True, "")

    def backup(self):
        retdata = dict()

        with open(LOGFILE_CONF_FILEPATH, "r") as file:
            raw_data = file.read()
            raw_data_bytes = raw_data.encode("utf-8")
            encoded_data = base64.b64encode(raw_data_bytes)
            encoded_str = encoded_data.decode("utf-8")
            retdata[RAW_DATA] = encoded_str

        retdata[APPEND_ATTR_DATA] = self.get_append_attr_data(SYSLOG_PATHS)
        retdata[ST_MODE] = self.get_mode_data(SYSLOG_PATHS)

        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        args = json.loads(args_json)

        if RAW_DATA in args:
            encoded_bytes = args[RAW_DATA].encode("utf-8")
            decoded_bytes = base64.b64decode(encoded_bytes)
            decoded_string = decoded_bytes.decode("utf-8")

            with open(LOGFILE_CONF_FILEPATH, "w") as file:
                file.write(decoded_string)
        else:  # 没有找到备份数据，报错
            return (False, "No backup data found(no raw-data field)")

        if APPEND_ATTR_DATA in args:
            self.set_append_attr_data(args[APPEND_ATTR_DATA])
        else:  # 没有找到备份数据，报错
            return (False, "No backup data found(no append-attr-data field)")

        if ST_MODE in args:
            self.set_mode_data(args[ST_MODE])
        else:  # 没有找到备份数据，报错
            return (False, "No backup data found(no st-mode field)")

        return (True, "")

    def change_mode(self, log_file, mode):
        has_append_attr = self.check_append_attr(log_file)
        # 先去掉追加属性，否则无法修改权限
        if has_append_attr:
            self.set_attr_limit(False, [log_file])

        os.chmod(log_file, mode)

        # 还原追加属性
        if has_append_attr:
            self.set_attr_limit(True, [log_file])

    # 日志权限限制
    def set_permissions_limit(self, permissions_limit, file_list):
        for log_file in file_list:
            if not os.access(log_file, os.F_OK):
                continue
            if permissions_limit:
                mode = os.stat(log_file).st_mode
                self.change_mode(log_file, mode & ~EXCLUDE_MODE)

    # 追加属性限制
    def set_attr_limit(self, append_attr_limit, file_list):
        for log_file in file_list:
            if not os.access(log_file, os.F_OK):
                continue
            if append_attr_limit:
                br.utils.subprocess_not_output("sudo chattr +a {}".format(log_file))
            else:
                br.utils.subprocess_not_output("sudo chattr -a {}".format(log_file))

    # 获取文件追加属性
    def get_append_attr_data(self, file_list):
        attr_data = ""
        for file in file_list:
            if not os.access(file, os.F_OK):
                continue
            attr = 1 if self.check_append_attr(file) else 0
            if len(attr_data) != 0:
                attr_data += ";"
            attr_data += file + ";" + str(attr)
        return attr_data

    # 设置文件追加属性
    def set_append_attr_data(self, attr_data):
        attr_info = attr_data.split(";")
        for i in range(0, len(attr_info), 2):
            if i + 1 < len(attr_info):
                file = attr_info[i]
                has_append_attr = attr_info[i + 1]

                if not os.access(file, os.F_OK):
                    continue

                if has_append_attr:
                    br.utils.subprocess_not_output("sudo chattr +a {}".format(file))
                else:
                    br.utils.subprocess_not_output("sudo chattr -a {}".format(file))

    # 检查是否有追加属性
    def check_append_attr(self, file):
        output = br.utils.subprocess_has_output(
            "lsattr {} | awk '{{print $1}}' | grep -o 'a'".format(file)
        )
        if len(output) == 0:
            return False
        else:
            return True

    def get_mode_data(self, file_list):
        st_mode = ""
        for file in file_list:
            if not os.access(file, os.F_OK):
                continue
            mode = os.stat(file).st_mode
            if st_mode:
                st_mode += ";"
            st_mode += file + ";" + str(mode)
        return st_mode

    def set_mode_data(self, mode_data):
        mode_info = mode_data.split(";")
        for i in range(0, len(mode_info), 2):
            if i + 1 < len(mode_info):
                if not os.access(mode_info[i], os.F_OK):
                    continue
                self.change_mode(mode_info[i], int(mode_info[i + 1]))
