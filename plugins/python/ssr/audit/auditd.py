#--coding:utf8 --

from ssr.systemd import SwitchBase
import json
import ssr.configuration
import ssr.log
import ssr.utils

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

    def is_rule_exist(self, path):
        output = ssr.utils.subprocess_has_output("auditctl -l | grep '{0}'".format(path[:-1]))
        if len(output) != 0:
            return True
        else:
            return False

    def get(self):
        retdata = dict()
        retdata[AUDIT_DEL_ALL_RULE_KEY] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        add_rules = "-w {0} {1}".format(args[AUDIT_ADD_PATH_KEY], AUDIT_RULE_TAIL)
        del_rules = "-w {0} {1}".format(args[AUDIT_DEL_PATH_KEY], AUDIT_RULE_TAIL)

        if args[AUDIT_ADD_PATH_KEY] != "" and not self.is_rule_exist(args[AUDIT_ADD_PATH_KEY]):
            self.conf.set_value("1=-w {0};2={1}".format(args[AUDIT_ADD_PATH_KEY], AUDIT_RULE_TAIL), add_rules)
        if args[AUDIT_DEL_PATH_KEY] != "":
            self.conf.del_record(del_rules)
        if args[AUDIT_DEL_ALL_RULE_KEY]:
            ssr.utils.subprocess_not_output("echo '' > {0}".format(AUDIT_RULES_PATH))
        
        ssr.utils.subprocess_not_output("augenrules --load")
        self.service.service_restart()
        return (True, '')