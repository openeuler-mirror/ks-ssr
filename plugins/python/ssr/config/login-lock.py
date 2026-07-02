#--coding:utf8 --

import json
import ssr.configuration

#检查系统版本
#对3.3和3.4做不同的处理
#对于需要插入配置的情况处理,未开启登录锁定.

LOGIN_LOCK_CONF_PATH = "/etc/pam.d/system-auth"
LOGIN_LOCK_CONF_KEY_TALLY = "auth        required                                     pam_tally.so"

LOGIN_LOCK_CONF_KEY_FAILURES = "deny"
LOGIN_LOCK_CONF_KEY_LOCK_TIME = "lock_time"
LOGIN_LOCK_CONF_KEY_UNLOCK_TIME = "unlock_time"
LOGIN_LOCK_CONF_KEY_ROOT_LOCK = "even_deny_root"
LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME = "root_unlock_time"

class LoginLock:
    def __init__(self):
        self.conf = ssr.configuration.PAM(LOGIN_LOCK_CONF_PATH, "auth\\s+required\\s+pam_tally.so")

    def get(self):
        retdata = dict()

        deny_value = self.conf.get_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=")
        lock_time = self.conf.get_value(LOGIN_LOCK_CONF_KEY_LOCK_TIME, "=")
        unlock_time = self.conf.get_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=")
        even_deny_root = self.conf.get_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
        root_unlock_time = self.conf.get_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=")

        retdata[LOGIN_LOCK_CONF_KEY_FAILURES] = int(deny_value)
        retdata[LOGIN_LOCK_CONF_KEY_LOCK_TIME] = int(lock_time)
        retdata[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME] = int(unlock_time)
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_LOCK] = bool(even_deny_root)
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME] = int(root_unlock_time)

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.conf.set_value(LOGIN_LOCK_CONF_KEY_LOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_LOCK_TIME], "=")
        self.conf.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.conf.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.conf.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        return (True, '')