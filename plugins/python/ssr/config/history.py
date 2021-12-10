#--coding:utf8 --

import json
import ssr.utils
import ssr.configuration

HISTORY_SIZE_LIMIT_CONF_PATH = "/etc/profile"
HISTORY_SIZE_LIMIT_CONF_KEY_HISTSIZE = "HISTSIZE"

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

        #设置后使得配置生效需要初始化profile文件
        command = 'source {0}'.format(HISTORY_SIZE_LIMIT_CONF_PATH)
        ssr.utils.subprocess_not_output(command)

        return (True, '')