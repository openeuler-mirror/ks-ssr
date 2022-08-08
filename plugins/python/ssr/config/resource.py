#--coding:utf8 --

import json
import re
import os
import ssr.utils
import ssr.configuration
import ssr.log

RESOURCE_LIMITS_CONF_PATH = "/etc/security/limits.d/90-ssr-config.conf"
SELINUX_MODULES_ULIMIT_PATH = "/usr/share/ks-ssr-manager/ssr-ulimit.pp"
PAM_CHECK_PATH = "/etc/security/limits.d/20-nproc.conf"

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
        self.conf_stack = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH)

    def get_selinux_status(self):
        output = ssr.utils.subprocess_has_output("getenforce")
        ssr.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False

    def get(self):
        retdata = dict()

        #忽略有注释的行
        stack_cmd = "{0}".format(RESOURCE_LIMITS_STACK_CMD)
        stack_output = ssr.utils.subprocess_has_output(stack_cmd)

        rss_cmd = "{0}".format(RESOURCE_LIMITS_RSS_CMD)
        rss_output = ssr.utils.subprocess_has_output(rss_cmd)

        stack = self.conf.get_value(RESOURCE_LIMITS_KEY_STACK_HARD)
        rss  = self.conf.get_value(RESOURCE_LIMITS_KEY_RSS_HARD)

        stack_file_output = ssr.utils.subprocess_has_output('grep -r \"unlimited\" {0} | grep stack'.format(RESOURCE_LIMITS_CONF_PATH))
        rss_file_ouput = ssr.utils.subprocess_has_output('grep -r \"unlimited\" {0} | grep rss'.format(RESOURCE_LIMITS_CONF_PATH))

        # stack = self.conf.get_value(RESOURCE_LIMITS_KEY_STACK_HARD)
        # rss  = self.conf.get_value(RESOURCE_LIMITS_KEY_RSS_HARD)
        ssr.log.debug(rss_output)
        ssr.log.debug(stack_output)

        if len(stack_file_output) != 0 or len(rss_file_ouput) != 0:
            retdata['enabled'] = False
        else:
            retdata['enabled'] = True

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        ssr.utils.subprocess_not_output('echo \' \' > {0}'.format(RESOURCE_LIMITS_CONF_PATH))

        if args['enabled']:
            if self.get_selinux_status():
                ssr.utils.subprocess_not_output("semodule -r ssr-ulimit &> /dev/null")
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, '8192')
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, '10240')
            if not os.path.exists(PAM_CHECK_PATH):
                ssr.utils.subprocess_not_output("sed -i '/{0}/d' {1}".format('ulimit', HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH))
                self.conf_stack.set_value(RESOURCE_LIMITS_STACK_CMD, '8192')
                self.conf_stack.set_value(RESOURCE_LIMITS_RSS_CMD, '10240')
        else:
            if self.get_selinux_status():
                ssr.utils.subprocess_not_output("semodule -i {0}".format(SELINUX_MODULES_ULIMIT_PATH))
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, 'unlimited')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, 'unlimited')
            if not os.path.exists(PAM_CHECK_PATH):
                ssr.utils.subprocess_not_output("sed -i '/{0}/d' {1}".format('ulimit', HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH))
                self.conf_stack.set_value(RESOURCE_LIMITS_STACK_CMD, 'unlimited')
                self.conf_stack.set_value(RESOURCE_LIMITS_RSS_CMD, 'unlimited')


        return (True, '')

class HistorySizeLimit:
    def __init__(self):
        self.conf = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_PATH, "=", "=")
        self.conf_profile = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH, "=", "=")
        self.conf_bashrc = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH, "=", "=")

    def get(self):
        retdata = dict()

        # 获取HISTSIZE的值
        get_ssr_config = self.conf.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_profile = self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_profile_export =  self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT)
        get_bashrc = self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_bashrc_export = self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT)

        if len(get_profile) == 0:
            get_profile = '0'
        if len(get_profile_export) == 0:
            get_profile_export = '0'
        if len(get_bashrc) == 0:
            get_bashrc = '0'
        if len(get_bashrc_export) == 0:
            get_bashrc_export = '0'

        ssr.log.debug('get_ssr_config',get_ssr_config,'get_profile = ',get_profile,'get_profile_export = ',get_profile_export,'get_bashrc = ',get_bashrc,'get_bashrc_export = ',get_bashrc_export)
        ssr.log.debug('int(get_profile)|int(get_profile_export)|int(get_bashrc_export)|int(get_bashrc) = ',int(get_profile)|int(get_profile_export)|int(get_bashrc_export)|int(get_bashrc))
        if int(get_ssr_config) == int(get_profile)|int(get_profile_export)|int(get_bashrc_export)|int(get_bashrc):
            retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = int(get_ssr_config)
            return (True, json.dumps(retdata))
        else:
            if int(get_profile)|int(get_profile_export)|int(get_bashrc_export)|int(get_bashrc) == 0:
                retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = int(get_ssr_config)
                return (True, json.dumps(retdata)) 
            retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = False
            return (False, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # /etc/profile 或 /etc/bashrc 存在值则修改，不存在不填加
        if self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE):
            self.conf_profile.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        if self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE):
            self.conf_bashrc.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # 如果存在export HISTSIZE的值才进行改动，不存在则不添加
        if self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT):
            self.conf_profile.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        if self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT):
            self.conf_bashrc.set_all_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # 使配置生效
        cmd = "source" + " " + HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH + " " + HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH + " " + HISTORY_SIZE_LIMIT_CONF_PATH
        limit_open_command = '{0}'.format(cmd)
        open_output = ssr.utils.subprocess_not_output(limit_open_command)

        return (True, '')
