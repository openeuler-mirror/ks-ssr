#--coding:utf8 --

import json
import ssr.configuration
import ssr.utils
#from gi.repository import Gio


SYSCTL_PATH = '/usr/sbin/sysctl'

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
        ssr.utils.subprocess_not_output('{0} --system'.format(SYSCTL_PATH))

        return (True, '')

# class KeyRebootSwitch:
#     def __init__(self):
#         self.gso = Gio.Settings("org.mate.SettingsDaemon.plugins.media-keys")
#         self.key = "power"

#     def status(self):
#         command = '{0} | grep masked'.format(COMPOSITE_KEY_REBOOT_STATUS_CMD)
#         output = ssr.utils.subprocess_has_output(command)

#         value = self.gso.get_string(self.key)
#         if value == "disabled":
#             return False
#         return True

#     def open(self):
#         command = '{0}'.format(COMPOSITE_KEY_REBOOT_ENABLE_CMD)
#         output = ssr.utils.subprocess_not_output(command)

#         self.gso.set_string(self.key, '<Control><Alt>Delete')

#     def close(self):
#         command = '{0}'.format(COMPOSITE_KEY_REBOOT_DISABLE_CMD)
#         output = ssr.utils.subprocess_not_output(command)

#         self.gso.set_string(self.key, 'disabled')

#     def get(self):
#         retdata = dict()
#         retdata['enabled'] = self.status()
#         return (True, json.dumps(retdata))

#     def set(self, args_json):
#         args = json.loads(args_json)

#         if args['enabled']:
#             self.open()
#         if not args['enabled']:
#             self.close()

#         return (True, '')


class KeyRebootSwitch:
    def get(self):
        retdata = dict()
        retdata['enabled'] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        # if args['enabled']:
        #     self.open()
        # if not args['enabled']:
        #     self.close()

        return (True, '')