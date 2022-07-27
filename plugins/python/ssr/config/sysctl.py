#--coding:utf8 --

import json
import ssr.configuration
import ssr.utils
import os

SYSCTL_PATH = '/usr/sbin/sysctl'
#修改schemas默认值
SCHEMAS_CONF_FILEPATH = "/usr/share/glib-2.0/schemas/98-ssr-config.gschema.override"
RELOAD_SCHEMAS_CMD = "glib-compile-schemas /usr/share/glib-2.0/schemas"

MODIFY_RULE_CLOSE = "[org.mate.SettingsDaemon.plugins.media-keys]\npower=\'\'"
MODIFY_RULE_OPEN = "[org.mate.SettingsDaemon.plugins.media-keys]\npower=\'<Control><Alt>Delete\'"

SAK_KEY_SWITCH_CONF_FILE = "/etc/sysctl.d/90-ssr-config.conf"
SAK_KEY_SWITCH_CONF_KEY_SYSRQ = "kernel.sysrq"
DMESG_SWITCH_CONF_KEY_SYSRQ = "kernel.dmesg_restrict"

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

class Dmesg:
    def __init__(self):
        self.conf = ssr.configuration.KV(SAK_KEY_SWITCH_CONF_FILE, "=", "=")

    def get(self):
        retdata = dict()
        retdata[DMESG_SWITCH_CONF_KEY_SYSRQ] = bool(self.conf.get_value(DMESG_SWITCH_CONF_KEY_SYSRQ) == "1")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if args[DMESG_SWITCH_CONF_KEY_SYSRQ]:
            value = 1
        else :
            value = 0
        self.conf.set_value(DMESG_SWITCH_CONF_KEY_SYSRQ, value)
        ssr.utils.subprocess_not_output('{0} --system'.format(SYSCTL_PATH))

        return (True, '')

class KeyRebootSwitch:
    def __init__(self):
        self.conf = ssr.configuration.Table(SCHEMAS_CONF_FILEPATH, ",\\s+")

    def reload_schemas(self):
        cmd = '{0}'.format(RELOAD_SCHEMAS_CMD)
        output = ssr.utils.subprocess_has_output(cmd)

    #判断文件是否存在
    def status_exist(self):
        command =  "ls /usr/lib/systemd/system/ |grep -wx ctrl-alt-del.target"
        cmd = '{0}'.format(command)
        output = ssr.utils.subprocess_has_output(cmd)
        return len(output) != 0

    def status(self):
        command = '{0} | grep masked'.format(COMPOSITE_KEY_REBOOT_STATUS_CMD)
        output = ssr.utils.subprocess_has_output(command)
        return len(output) == 0

    #判断.bak是否存在
    def status_bak(self):
        command = " ls /usr/lib/systemd/system/ |grep ctrl-alt-del.target.bak"
        cmd = '{0}'.format(command)
        output = ssr.utils.subprocess_has_output(cmd)
        return len(output) != 0

    def open(self):
        command = '{0}'.format(COMPOSITE_KEY_REBOOT_ENABLE_CMD)
        output = ssr.utils.subprocess_not_output(command)
        if not os.path.exists('/etc/systemd/system/ctrl-alt-del.target'):
            ssr.utils.subprocess_not_output("ln -s /usr/lib/systemd/system/reboot.target /etc/systemd/system/ctrl-alt-del.target")

    def close(self):
        if  os.path.exists('/etc/systemd/system/ctrl-alt-del.target'):
            ssr.utils.subprocess_not_output("rm -rf /etc/systemd/system/ctrl-alt-del.target")
        command = '{0}'.format(COMPOSITE_KEY_REBOOT_DISABLE_CMD)
        output = ssr.utils.subprocess_not_output(command)

    def get(self):
        retdata = dict()
        if self.status_exist():
            retdata['enabled'] = self.status()
        if self.status_bak():
            retdata['enabled'] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if self.status_exist():
            if  not self.status() :
                if args['enabled']:
                    self.open()
                    rm_cmd = 'rm -rf {0}'.format(SCHEMAS_CONF_FILEPATH)
                    output = ssr.utils.subprocess_not_output(rm_cmd)
                    self.conf.set_value("1=[org.mate.SettingsDaemon.plugins.media-keys]\npower=\'\'", MODIFY_RULE_OPEN)
                    self.reload_schemas()
            else:
                if not args['enabled']:
                    self.close()
                    rm_cmd = 'rm -rf {0}'.format(SCHEMAS_CONF_FILEPATH)
                    output = ssr.utils.subprocess_not_output(rm_cmd)
                    self.conf.set_value("1=[org.mate.SettingsDaemon.plugins.media-keys]\npower=\'<Control><Alt>Delete\'", MODIFY_RULE_CLOSE)
                    self.reload_schemas()
        else:
            #针对3.3-6的处理规则，文件不存在，开关为打开是，将.bak改为ctrl-alt-del.target
            if args['enabled'] and self.status_bak():
                command = "mv /usr/lib/systemd/system/ctrl-alt-del.target.bak /usr/lib/systemd/system/ctrl-alt-del.target"
                output = ssr.utils.subprocess_not_output(command)

        return (True, '')