#--coding:utf8 --

import json
import ssr.configuration

PROFILE_CONF_PATH = "/etc/ssh/sshd_config"

# 会话登录后超过指定时间未操作则自动退出
#PROFILE_ARG_TMOUT = "tmout"

PROFILE_CLIENT_TMOUT = "ClientAliveInterval"
PROFILE_CLIENT_COUNT = "ClientAliveCountMax"


class SessionTimeout:
    def __init__(self):
        self.conf = ssr.configuration.KV(PROFILE_CONF_PATH, "", "=")

    def get(self):
        retdata = dict()
        tmout = self.conf.get_value(PROFILE_CLIENT_TMOUT)
        count = self.conf.get_value(PROFILE_CLIENT_COUNT)

        retdata[PROFILE_CLIENT_TMOUT] = tmout
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if (args[PROFILE_CLIENT_TMOUT] <= 0):
            self.conf.del_record(PROFILE_CLIENT_TMOUT)
            self.conf.del_record(PROFILE_CLIENT_COUNT)
        else:
            self.conf.set_value(PROFILE_CLIENT_TMOUT, args[PROFILE_CLIENT_TMOUT])
            self.conf.set_value(PROFILE_CLIENT_COUNT, 0)
        return (True, '')
