#--coding:utf8 --

import json
import ssr.configuration

PROFILE_CONF_PATH = "/etc/profile.d/ssr-external.sh"

# 会话登录后超过指定时间未操作则自动退出
PROFILE_ARG_TMOUT = "tmout"


class SessionTimeout:
    def __init__(self):
        self.conf = ssr.configuration.KV(PROFILE_CONF_PATH, "=", "=")

    def get(self):
        retdata = dict()
        tmout = self.conf.get_value("TMOUT")

        retdata[PROFILE_ARG_TMOUT] = (0 if len(tmout) == 0 else int(tmout))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if (args[PROFILE_ARG_TMOUT] <= 0):
            self.conf.del_record("TMOUT")
        else:
            self.conf.set_value("TMOUT", args[PROFILE_ARG_TMOUT])
        return (True, '')
