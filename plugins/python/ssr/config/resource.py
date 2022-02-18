#--coding:utf8 --

import json
import re
import ssr.utils
import ssr.configuration

RESOURCE_LIMITS_CONF_PATH = "/etc/security/limits.d/90-ssr-config.conf"
RESOURCE_LIMITS_KEY_STACK  = "*\s+soft\s+stack"
RESOURCE_LIMITS_KEY_RSS = "*\s+hard\s+rss"

HISTORY_SIZE_LIMIT_CONF_PATH = "/etc/profile.d/ssr-config.sh"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE = "HISTSIZE"

class ResourceLimits:
    def get(self):
        retdata = dict()

        #忽略有注释的行
        stack_cmd = 'grep -v "^\s*#" {0} | egrep "{1}"'.format(RESOURCE_LIMITS_CONF_PATH, RESOURCE_LIMITS_KEY_STACK)
        stack_output = ssr.utils.subprocess_has_output(stack_cmd)

        rss_cmd = 'grep -v "^\s*#" {0} | egrep "{1}"'.format(RESOURCE_LIMITS_CONF_PATH, RESOURCE_LIMITS_KEY_RSS)
        rss_output = ssr.utils.subprocess_has_output(rss_cmd)

        if len(stack_output) == 0:
            retdata['stack'] = ''
        else:
            stack = list(map(int,re.findall("\d+", stack_output)))
            retdata['stack'] = stack[0]

        if len(rss_output) == 0:
            retdata['rss'] = ''
        else:
            rss = list(map(int,re.findall("\d+", rss_output)))
            retdata['rss']  = rss[0]

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        stack_cmd = 'grep -v "^\s*#" {0} | egrep "{1}"'.format(RESOURCE_LIMITS_CONF_PATH, RESOURCE_LIMITS_KEY_STACK)
        stack_output = ssr.utils.subprocess_has_output(stack_cmd)

        rss_cmd = 'grep -v "^\s*#" {0} | egrep "{1}"'.format(RESOURCE_LIMITS_CONF_PATH, RESOURCE_LIMITS_KEY_RSS)
        rss_output = ssr.utils.subprocess_has_output(rss_cmd)

        if len(stack_output)==0:
            stack_cmd = 'echo \'*     soft    stack   {0}\' >> {1}'.format(args['stack'], RESOURCE_LIMITS_CONF_PATH)
        else:
            stack_cmd = 'sed -i \'s/{0}/*     soft    stack   {1}/g\' {2}'.format(stack_output, args['stack'], RESOURCE_LIMITS_CONF_PATH)

        if len(rss_output)==0:
            rss_cmd = 'echo \'*     hard    rss   {0}\' >> {1}'.format(args['rss'], RESOURCE_LIMITS_CONF_PATH)
        else:
            rss_cmd = 'sed -i \'s/{0}/*     hard    rss   {1}/g\' {2}'.format(rss_output, args['rss'], RESOURCE_LIMITS_CONF_PATH)

        ssr.utils.subprocess_not_output(stack_cmd)
        ssr.utils.subprocess_not_output(rss_cmd)

        return (True, '')

class HistorySizeLimit:
    def __init__(self):
        self.conf = ssr.configuration.KV(HISTORY_SIZE_LIMIT_CONF_PATH, "=", "=")

    def get(self):
        retdata = dict()
        retdata[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE] = int(self.conf.get_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value(HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE, args[HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE])

        return (True, '')
