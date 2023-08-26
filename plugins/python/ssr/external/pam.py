#--coding:utf8 --

import json
import ssr.configuration

SU_PAM_CONF_PATH = "/etc/pam.d/su"

# 禁止wheel组以外的用户su root
SU_WHEEL_ARG_ENABLED = "enabled"
# 如果没有匹配到对应的行，则使用该默认行进行设置
SU_WHEEL_FALLBACK_LINE = "auth           required        pam_wheel.so use_uid"

SU_WHEEL_NEXT_MATCH_LINE = "auth\\s+substack\\s+system-auth"

class SuWheel:
    def __init__(self):
        self.conf = ssr.configuration.PAM(SU_PAM_CONF_PATH, "auth\\s+required\\s+pam_wheel.so")

    def get(self):
        retdata = dict()
        retdata[SU_WHEEL_ARG_ENABLED] = (len(self.conf.get_line()) != 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if (args[SU_WHEEL_ARG_ENABLED]):
            self.conf.set_line(SU_WHEEL_FALLBACK_LINE, SU_WHEEL_NEXT_MATCH_LINE)
        else:
            self.conf.del_line()
        return (True, '')