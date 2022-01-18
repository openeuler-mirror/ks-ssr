#--coding:utf8 --

import json
import ssr.configuration
import ssr.utils

SAK_KEY_SWITCH_CONF_FILE = "/etc/sysctl.d/90-ssr-config.conf"
SAK_KEY_SWITCH_CONF_KEY_SYSRQ = "kernel.sysrq"

COMPOSITE_KEY_REBOOT_STATUS_CMD = "systemctl   status  ctrl-alt-del.target"
COMPOSITE_KEY_REBOOT_DISABLE_CMD = "systemctl   mask   ctrl-alt-del.target"
COMPOSITE_KEY_REBOOT_ENABLE_CMD = "systemctl   unmask   ctrl-alt-del.target"

class SAKKey:
    def __init__(self):
        self.conf = ssr.configuration.KV(SAK_KEY_SWITCH_CONF_FILE, "=", "=")

    def get(self):
        retdata = dict()
        retdata[SAK_KEY_SWITCH_CONF_KEY_SYSRQ] = bool(self.conf.get_value(SAK_KEY_SWITCH_CONF_KEY_SYSRQ) == "1")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if args[SAK_KEY_SWITCH_CONF_KEY_SYSRQ]:
            value = 1
        else :
            value = 0
        self.conf.set_value(SAK_KEY_SWITCH_CONF_KEY_SYSRQ, value)

        return (True, '')

class KeyRebootSwitch:
    def status(self):
        command = '{0} | grep masked'.format(COMPOSITE_KEY_REBOOT_STATUS_CMD)
        output = ssr.utils.subprocess_has_output(command)
        return len(output) == 0

    def open(self):
        command = '{0}'.format(COMPOSITE_KEY_REBOOT_ENABLE_CMD)
        output = ssr.utils.subprocess_not_output(command)

    def close(self):
        command = '{0}'.format(COMPOSITE_KEY_REBOOT_DISABLE_CMD)
        output = ssr.utils.subprocess_not_output(command)

    def get(self):
        retdata = dict()
        retdata['enabled'] = self.status()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args['enabled']:
            self.open()
        else:
            self.close()

        return (True, '')
