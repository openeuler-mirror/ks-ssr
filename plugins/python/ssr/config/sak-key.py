#--coding:utf8 --

import json
import ssr.configuration

SAK_KEY_SWITCH_CONF_PATH = "/etc/sysctl.conf"
SAK_KEY_SWITCH_CONF_KEY_SYSRQ = "kernel.sysrq"

class SAKKey:
    def __init__(self):
        self.conf = ssr.configuration.KV(SAK_KEY_SWITCH_CONF_PATH, "=", "=")

    def get(self):
        retdata = dict()
        retdata[SAK_KEY_SWITCH_CONF_KEY_SYSRQ] = bool(self.conf.get_value(SAK_KEY_SWITCH_CONF_KEY_SYSRQ) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if args[SAK_KEY_SWITCH_CONF_KEY_SYSRQ]:
            value = 1
        else :
            value = 0
        self.conf.set_value(SAK_KEY_SWITCH_CONF_KEY_SYSRQ, value)

        return (True, '')