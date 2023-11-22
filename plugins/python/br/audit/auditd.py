# -*- coding: utf-8 -*-

from br.systemd import SwitchBase
import json
import os
import br.configuration
import br.log
import br.utils

AUDIT_RULES_PATH = "/etc/audit/rules.d/br-audit.rules"

AUDIT_ADD_PATH_KEY = "add-path"
AUDIT_DEL_PATH_KEY = "del-path"
AUDIT_DEL_ALL_RULE_KEY = "enabled"

AUDIT_RULE_TAIL = "-p rwxa"

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

        output_result = str(br.utils.subprocess_has_output("auditctl -l"))
        for line in output_result.splitlines():
            if line.split()[1].strip() == path:
                return True
        return False

    def get(self):
        retdata = dict()
        retdata[AUDIT_DEL_ALL_RULE_KEY] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        add_rules = "-w {0} {1}".format(args[AUDIT_ADD_PATH_KEY],
                                        AUDIT_RULE_TAIL)
        del_rules = "-w {0} {1}".format(args[AUDIT_DEL_PATH_KEY],
                                        AUDIT_RULE_TAIL)

        is_arg_empty_add = args[AUDIT_ADD_PATH_KEY] != "" and args[AUDIT_ADD_PATH_KEY] != "\"\""
        is_arg_empty_del = args[AUDIT_DEL_PATH_KEY] != "" and args[AUDIT_DEL_PATH_KEY] != "\"\""
        # Need to close selinux for use.
        if ((is_arg_empty_add and self.get_selinux_status()) or
                (is_arg_empty_del and self.get_selinux_status())):
            return (False, "Please close SELinux and use it!")
        # No such file or directory.
        if ((is_arg_empty_add and not os.path.exists(args[AUDIT_ADD_PATH_KEY])) or
                (is_arg_empty_del and not os.path.exists(args[AUDIT_DEL_PATH_KEY]))):
            return (False, "No such file or directory.")

        if is_arg_empty_add and not self.is_rule_exist(args[AUDIT_ADD_PATH_KEY]):
            self.conf.set_value(
                "1=-w {0};2={1}".format(args[AUDIT_ADD_PATH_KEY], AUDIT_RULE_TAIL), add_rules)
        if is_arg_empty_del:
            self.conf.del_record(del_rules)
        if args[AUDIT_DEL_ALL_RULE_KEY]:
            br.utils.subprocess_not_output(
                "echo '' > {0}".format(AUDIT_RULES_PATH))

        br.utils.subprocess_not_output("augenrules --load")
        self.service.service_restart()
        return (True, '')
