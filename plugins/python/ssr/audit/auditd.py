# -*- coding: utf-8 -*-

from ssr.systemd import SwitchBase
import json
import os
import ssr.configuration
import ssr.log
import ssr.utils
from ssr.translation import _

AUDIT_RULES_PATH = "/etc/audit/rules.d/ssr-audit.rules"

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
        self.conf = ssr.configuration.Table(AUDIT_RULES_PATH, ",\\s+")
        self.service = ssr.systemd.Proxy("auditd")

    def get_selinux_status(self):
        output = ssr.utils.subprocess_has_output("getenforce")
        ssr.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False

    def is_rule_exist(self, path):
        if path[-1] == '/':
            path = path[:-1]

        output_result = str(ssr.utils.subprocess_has_output("auditctl -l"))
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
        # Need to close selinux for use.
        if ((args[AUDIT_ADD_PATH_KEY] != "" and self.get_selinux_status()) or
                (args[AUDIT_DEL_PATH_KEY] != "" and self.get_selinux_status())):
            return (False, _("Please close SELinux and use it!\t"))
        # No such file or directory.
        if ((args[AUDIT_ADD_PATH_KEY] != "" and not os.path.exists(args[AUDIT_ADD_PATH_KEY])) or
                (args[AUDIT_DEL_PATH_KEY] != "" and not os.path.exists(args[AUDIT_DEL_PATH_KEY]))):
            return (False, _("No such file or directory\t"))

        if args[AUDIT_ADD_PATH_KEY] != "" and not self.is_rule_exist(args[AUDIT_ADD_PATH_KEY]):
            self.conf.set_value(
                "1=-w {0};2={1}".format(args[AUDIT_ADD_PATH_KEY], AUDIT_RULE_TAIL), add_rules)
        if args[AUDIT_DEL_PATH_KEY] != "":
            self.conf.del_record(del_rules)
        if args[AUDIT_DEL_ALL_RULE_KEY]:
            ssr.utils.subprocess_not_output(
                "echo '' > {0}".format(AUDIT_RULES_PATH))

        ssr.utils.subprocess_not_output("augenrules --load")
        self.service.service_restart()
        return (True, '')
