# -*- coding: utf-8 -*-

from br.systemd import SwitchBase
import json
import os
import re
import br.configuration
import br.log
import br.utils

AUDIT_RULES_PATH = "/etc/audit/rules.d/br-audit.rules"
AUDIT_DEL_CMD = "auditctl -W"

AUDIT_ADD_PATH_KEY = "add-path"
AUDIT_DEL_PATH_KEY = "del-path"
AUDIT_DEL_ALL_RULE_KEY = "enabled"

AUDIT_RULE_TAIL = "-p rwxa"
NULL_STR = "null"

# 系统审计服务


class Switch(SwitchBase):
    def __init__(self):
        super(Switch, self).__init__('auditd')


class Rules():
    def __init__(self):
        self.conf = br.configuration.Table(AUDIT_RULES_PATH, ",\\s+")
        self.service = br.systemd.Proxy("auditd")

    def get_selinux_status(self):
        output = br.utils.subprocess_has_output("getenforce")
        br.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False

    def is_rule_exist(self, path):
        if path[-1] == '/':
            path = path[:-1]
        
        value = "-w " + path + " " + AUDIT_RULE_TAIL
        output_result = str(br.utils.subprocess_has_output("auditctl -l"))
        for line in output_result.splitlines():
            if line.strip() == value:
                return True
        return False
    # TODO 后续在ks-br-config添加一个line类型去实现行匹配和行删除
    def delete_line(self, del_line):
        # 读取文件所有行
        with open(AUDIT_RULES_PATH, "r") as file:
            lines = file.readlines()
        
        # 遍历行，去除包含line这一行
        new_lines = []
        for line in lines:
            if line.strip() == del_line:
                continue
            new_lines.append(line)

        # 写入文件
        with open(AUDIT_RULES_PATH, "w") as file:
            file.writelines(new_lines)
    def delete_all_lines(self):
        # 读取文件所有行
        with open(AUDIT_RULES_PATH, "r") as file:
            lines = file.readlines()
        # 遍历行
        for line in lines:
            br.utils.subprocess_not_output("{0} {1}".format(AUDIT_DEL_CMD, line.replace("-w ", "")))
        br.utils.subprocess_not_output("cat /dev/null > {0}".format(AUDIT_RULES_PATH))

    def set_rule(self, is_arg_empty_add, is_arg_empty_del, add_rule_key, add_rules, del_rules, del_all_arg):
        if is_arg_empty_add and not self.is_rule_exist(add_rule_key):
            self.conf.set_value(
                "1=-w\ {0}".format(add_rule_key), add_rules)
            br.utils.subprocess_not_output("auditctl {0}".format(add_rules))
        if is_arg_empty_del:
            self.delete_line(del_rules)
            br.utils.subprocess_has_output_ignore_error_handling("{0} {1}".format(AUDIT_DEL_CMD, del_rules.replace("-w ", "")))
        if del_all_arg:
            self.delete_all_lines()
        # 忽略这个错误，重复执行这条命令也会抛出错误
        br.utils.subprocess_has_output_ignore_error_handling("augenrules --load")
        self.service.service_reload()

    def get(self):
        retdata = dict()
        retdata[AUDIT_DEL_ALL_RULE_KEY] = False
        if os.path.getsize(AUDIT_RULES_PATH) == 0:
            retdata[AUDIT_ADD_PATH_KEY] = NULL_STR
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        add_rule_key = ""
        if args[AUDIT_ADD_PATH_KEY] != NULL_STR:
            add_rule_key = args[AUDIT_ADD_PATH_KEY]
        else:
            self.delete_all_lines()
        add_rules = "-w {0} {1}".format(add_rule_key,
                                        AUDIT_RULE_TAIL)
        del_rules = "-w {0} {1}".format(args[AUDIT_DEL_PATH_KEY],
                                        AUDIT_RULE_TAIL)

        is_arg_empty_add = add_rule_key != "" and add_rule_key != "\"\""
        is_arg_empty_del = args[AUDIT_DEL_PATH_KEY] != "" and args[AUDIT_DEL_PATH_KEY] != "\"\""
        # Need to close selinux for use.
        if ((is_arg_empty_add and self.get_selinux_status()) or
                (is_arg_empty_del and self.get_selinux_status())):
            return (False, "Please close SELinux and use it!")
        # No such file or directory.
        if ((is_arg_empty_add and not os.path.exists(add_rule_key)) or
                (is_arg_empty_del and not os.path.exists(args[AUDIT_DEL_PATH_KEY]))):
            return (False, "No such file or directory.")
        self.set_rule(is_arg_empty_add, is_arg_empty_del, add_rule_key, add_rules, del_rules, args[AUDIT_DEL_ALL_RULE_KEY])
        return (True, '')
