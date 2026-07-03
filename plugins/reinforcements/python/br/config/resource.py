# -*- coding: utf-8 -*-

import json
import re
import os
import br.utils
import br.configuration
import br.log

RESOURCE_LIMITS_CONF_PATH = "/etc/security/limits.d/90-br-config.conf"
SELINUX_MODULES_ULIMIT_PATH = "/usr/share/ks-ssr/br-ulimit.pp"
PAM_CHECK_PATH = "/etc/security/limits.d/20-nproc.conf"

RESOURCE_LIMITS_KEY_STACK_SOFT = "*                soft    stack"
RESOURCE_LIMITS_KEY_STACK_HARD = "*                hard    stack"
RESOURCE_LIMITS_KEY_RSS_SOFT = "*                soft    rss"
RESOURCE_LIMITS_KEY_RSS_HARD = "*                hard    rss"

RESOURCE_LIMITS_STACK_CMD = "ulimit -s"
RESOURCE_LIMITS_SOFT_STACK_CMD = "ulimit -Ss"
RESOURCE_LIMITS_HARD_STACK_CMD = "ulimit -Hs"
RESOURCE_LIMITS_RSS_CMD = "ulimit -m"
RESOURCE_LIMITS_UNLIMITED = "unlimited"
RESOURCE_LIMITS_RSS_KEY = "enabled"

HISTORY_SIZE_LIMIT_CONF_PATH = "/etc/profile.d/br-config.sh"
HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH = "/etc/profile"
HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH = "/etc/bashrc"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE = "HISTSIZE"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT = "export HISTSIZE"


class ResourceLimits:
    def __init__(self):
        self.conf = br.configuration.KV(RESOURCE_LIMITS_CONF_PATH)
        self.conf_stack = br.configuration.KV(
            HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH)
        self.format_str = "sed -i '/{0}/d' {1}"

    def get_selinux_status(self):
        output = br.utils.subprocess_has_output("getenforce")
        br.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False

    def reset(self):
        br.utils.subprocess_not_output('cat /dev/null > {0}'.format(RESOURCE_LIMITS_CONF_PATH))
        if not os.path.exists(PAM_CHECK_PATH) or self.get_selinux_status():
            br.utils.subprocess_not_output(self.format_str.format('ulimit', HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH))
            self.conf_stack.set_value(RESOURCE_LIMITS_SOFT_STACK_CMD, '')
            self.conf_stack.set_value(RESOURCE_LIMITS_HARD_STACK_CMD, '')
            self.conf_stack.set_value(RESOURCE_LIMITS_RSS_CMD, '')

    def get(self):
        retdata = dict()

        # 忽略有注释的行
        stack_output = br.utils.subprocess_has_output(RESOURCE_LIMITS_STACK_CMD)

        rss_cmd = "{0}".format(RESOURCE_LIMITS_RSS_CMD)
        rss_output = br.utils.subprocess_has_output(rss_cmd)

        stack_file_output = br.utils.subprocess_has_output(
            'grep -r \"unlimited\" {0} | grep stack'.format(RESOURCE_LIMITS_CONF_PATH))
        rss_file_output = br.utils.subprocess_has_output(
            'grep -r \"unlimited\" {0} | grep rss'.format(RESOURCE_LIMITS_CONF_PATH))
        br.log.debug(rss_output)
        br.log.debug(stack_output)
        if os.path.getsize(RESOURCE_LIMITS_CONF_PATH) == 0:
            retdata[RESOURCE_LIMITS_RSS_KEY] = ""
            return (True, json.dumps(retdata))

        if len(stack_file_output) != 0 or len(rss_file_output) != 0:
            retdata[RESOURCE_LIMITS_RSS_KEY] = False
        else:
            retdata[RESOURCE_LIMITS_RSS_KEY] = True

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if not str(args[RESOURCE_LIMITS_RSS_KEY]):
            self.reset()
            return (True, '')

        br.utils.subprocess_not_output('cat /dev/null > {0}'.format(RESOURCE_LIMITS_CONF_PATH))
        if args[RESOURCE_LIMITS_RSS_KEY]:
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, '8192')
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, '10240')
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, '10240')
            if not os.path.exists(PAM_CHECK_PATH) or self.get_selinux_status():
                br.utils.subprocess_not_output(
                    self.format_str.format('ulimit', HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH))
                self.conf_stack.set_value(RESOURCE_LIMITS_SOFT_STACK_CMD, '8192')
                self.conf_stack.set_value(RESOURCE_LIMITS_HARD_STACK_CMD, '10240')
                self.conf_stack.set_value(RESOURCE_LIMITS_RSS_CMD, '10240')
        else:
            if self.get_selinux_status() and len(br.utils.subprocess_has_output("semodule -l |grep br-ulimit")) == 0:
                br.utils.subprocess_not_output(
                    "semodule -i {0}".format(SELINUX_MODULES_ULIMIT_PATH))
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_SOFT, RESOURCE_LIMITS_UNLIMITED)
            self.conf.set_value(RESOURCE_LIMITS_KEY_STACK_HARD, RESOURCE_LIMITS_UNLIMITED)
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_SOFT, RESOURCE_LIMITS_UNLIMITED)
            self.conf.set_value(RESOURCE_LIMITS_KEY_RSS_HARD, RESOURCE_LIMITS_UNLIMITED)
            if not os.path.exists(PAM_CHECK_PATH) or self.get_selinux_status():
                br.utils.subprocess_not_output(
                    self.format_str.format('ulimit', HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH))
                self.conf_stack.set_value(RESOURCE_LIMITS_SOFT_STACK_CMD, RESOURCE_LIMITS_UNLIMITED)
                self.conf_stack.set_value(RESOURCE_LIMITS_HARD_STACK_CMD, RESOURCE_LIMITS_UNLIMITED)
                self.conf_stack.set_value(RESOURCE_LIMITS_RSS_CMD, RESOURCE_LIMITS_UNLIMITED)

        return (True, '')


class HistorySizeLimit:
    def __init__(self):
        self.conf = br.configuration.KV(
            HISTORY_SIZE_LIMIT_CONF_PATH, "=", "=")
        self.conf_profile = br.configuration.KV(
            HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH, "=", "=")
        self.conf_bashrc = br.configuration.KV(
            HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH, "=", "=")

    def get(self):
        retdata = dict()

        # 获取HISTSIZE的值
        get_ssr_config = self.conf.get_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_profile = self.conf_profile.get_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_profile_export = self.conf_profile.get_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT)
        get_bashrc = self.conf_bashrc.get_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE)
        get_bashrc_export = self.conf_bashrc.get_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT)

        if not get_ssr_config:
            get_ssr_config='0'
        if not get_profile:
            get_profile='0'
        if not get_profile_export:
            get_profile_export='0'
        if not get_bashrc:
            get_bashrc='0'
        if not get_bashrc_export:
            get_bashrc_export='0'

        br.log.debug('get_ssr_config', int(get_ssr_config), 'get_profile = ', int(get_profile), 'get_profile_export = ',
                      int(get_profile_export), 'get_bashrc = ', int(get_bashrc), 'get_bashrc_export = ', int(get_bashrc_export))
        value=int(get_ssr_config) | int(get_profile) | int(get_profile_export) | int(get_bashrc_export) | int(get_bashrc)
        br.log.debug('int(get_profile)|int(get_profile_export)|int(get_bashrc_export)|int(get_bashrc)', value)
        retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = value
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_all_value(
            HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # /etc/profile 或 /etc/bashrc 存在值则修改，不存在不填加
        if self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE):
            self.conf_profile.set_all_value(
                HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        if self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE):
            self.conf_bashrc.set_all_value(
                HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # 如果存在export HISTSIZE的值才进行改动，不存在则不添加
        if self.conf_profile.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT):
            self.conf_profile.set_all_value(
                HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        if self.conf_bashrc.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT):
            self.conf_bashrc.set_all_value(
                HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE_EXPORT, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])
        # 使配置生效
        cmd = "source" + " " + HISTORY_SIZE_LIMIT_CONF_BASHRC_PATH + " " + \
            HISTORY_SIZE_LIMIT_CONF_PROFILE_PATH + " " + HISTORY_SIZE_LIMIT_CONF_PATH
        limit_open_command = '{0}'.format(cmd)
        br.utils.subprocess_not_output(limit_open_command)

        return (True, '')
