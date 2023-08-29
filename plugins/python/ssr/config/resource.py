#--coding:utf8 --

import json
import re
import ssr.utils
import ssr.configuration

RESOURCE_LIMITS_CONF_PATH = "/etc/security/limits.d/90-ssr-config.conf"
RESOURCE_LIMITS_KEY_STACK_SOFT  = "*                soft    stack"
RESOURCE_LIMITS_KEY_STACK_HARD  = "*                hard    stack"
RESOURCE_LIMITS_KEY_RSS_SOFT = "*                soft    rss"
RESOURCE_LIMITS_KEY_RSS_HARD = "*                hard    rss"

RESOURCE_LIMITS_STACK_CMD = "ulimit -s"
RESOURCE_LIMITS_RSS_CMD = "ulimit -m"

# RESOURCE_LIMITS_KEY_STACK  = "*stack"
# RESOURCE_LIMITS_KEY_RSS = "*rss"


HISTORY_SIZE_LIMIT_CONF_PATH = "/etc/profile.d/ssr-config.sh"
HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH = "/etc/profile"
HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH = "/etc/bashrc"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE = "HISTSIZE"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT = "export HISTSIZE"

class ResourceLimits:
    def __init__(self):
        self.conf = ssr.configuration.KV(RESOURCE_LIMITS_CONF_PATH)

    def get(self):
        retdata = dict()

        #忽略有注释的行
        stack_cmd = "{0}".format(RESOURCE_LIMITS_STACK_CMD)
        stack_output = ssr.utils.subprocess_has_output(stack_cmd)

        rss_cmd = "{0}".format(RESOURCE_LIMITS_RSS_CMD)
        rss_output = ssr.utils.subprocess_has_output(rss_cmd)

        stack = self.conf.get_value(RESOURCE_LIMITS_KEY_STACK_HARD)
        rss  = self.conf.get_value(RESOURCE_LIMITS_KEY_RSS_HARD)

        if (stack_cmd == "unlimited" and rss_output == "unlimited") and (stack == "unlimited" and rss == "unlimited"):
            retdata['enabled'] = False
        else:
            retdata['enabled'] = True

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.del_record(RESOURCE_LIMITS_KEY_STACK_SOFT)
        self.conf.del_record(RESOURCE_LIMITS_KEY_STACK_HARD)
        self.conf.del_record(RESOURCE_LIMITS_KEY_RSS_SOFT)
        self.conf.del_record(RESOURCE_LIMITS_KEY_RSS_HARD)

        if args['enabled']:
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, '8190')
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, '10240')
        else:
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, 'unlimited')

        return (True, '')

class HistorySizeLimit:
    def __init__(self):
        self.conf = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_PATH, "=", "=")
        self.conf_profile = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH, "=", "=")
        self.conf_bashrc = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH, "=", "=")

    def get(self):
        retdata = dict()
        # 三个文件的 HISTSIZE值相等，且值为默认值才为加固成功
        if int(self.conf.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)) == int(self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)) and int(self.conf.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)) == int(self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)):
            retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = int(self.conf.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE))
        else:
            retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = False

        retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT] =  int(self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)) 
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        self.conf_profile.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        self.conf_bashrc.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        self.conf_bashrc.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])

        # 使配置生效
        cmd = "source" + " " + HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH + " " + HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH + " " + HISTORY_SIZE_LIMIT_CONF_PATH
        limit_open_command = '{0}'.format(cmd)
        open_output = ssr.utils.subprocess_not_output(limit_open_command)

        return (True, '')
