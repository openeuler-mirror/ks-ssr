#--coding:utf8 --

import json
import ssr.configuration

#检查系统版本
#对3.3和3.4做不同的处理
#对于需要插入配置的情况处理,未开启登录锁定.

LOGIN_LOCK_SYSTEM_CONF_PATH = "/etc/pam.d/system-auth"
LOGIN_LOCK_PASSWORD_CONF_PATH = "/etc/pam.d/password-auth"
LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH = "auth        requisite                                    pam_faillock.so preauth audit"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL = "auth        [default=die]                                pam_faillock.so authfail audit"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC = "auth        sufficient                                   pam_faillock.so authsucc audit"

LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX = "auth\\s+requisite\\s+pam_faillock.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX = "auth\\s+\[default=die\]\\s+pam_faillock.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX = "auth\\s+sufficient\\s+pam_faillock.so"

LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN = "auth\\s+required\\s+pam_faildelay.so"
LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN = "auth\\s+requisite\\s+pam_succeed_if.so"
LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN = "auth\\s+requisite\\s+pam_succeed_if.so"

LOGIN_LOCK_CONF_KEY_FAILURES = "deny"
LOGIN_LOCK_CONF_KEY_UNLOCK_TIME = "unlock_time"
LOGIN_LOCK_CONF_KEY_ROOT_LOCK = "even_deny_root"
LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME = "root_unlock_time"

class LoginLock:
    def __init__(self):
        self.system_faillock_preauth = ssr.configuration.PAM(LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX)
        self.system_faillock_authfail = ssr.configuration.PAM(LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX)
        self.system_faillock_authsucc = ssr.configuration.PAM(LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX)

        self.password_faillock_preauth = ssr.configuration.PAM(LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX)
        self.password_faillock_authfail = ssr.configuration.PAM(LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX)
        self.password_faillock_authsucc = ssr.configuration.PAM(LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX)

    def get(self):
        retdata = dict()

        deny_value = self.system_faillock_authfail.get_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=")
        unlock_time = self.system_faillock_authfail.get_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=")
        even_deny_root = self.system_faillock_authfail.get_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
        root_unlock_time = self.system_faillock_authfail.get_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=")

        retdata[LOGIN_LOCK_CONF_KEY_FAILURES] = int(deny_value)
        retdata[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME] = int(unlock_time)
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_LOCK] = even_deny_root != False
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME] = int(root_unlock_time)

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        ##system-auth
        if len(self.system_faillock_preauth.get_line()) == 0:
            self.system_faillock_preauth.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH, LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN)

        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        if len(self.system_faillock_authfail.get_line()) == 0:
            self.system_faillock_authfail.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL, LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN)

        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        if len(self.system_faillock_authsucc.get_line()) == 0:
            self.system_faillock_authsucc.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC, LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN)

        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        ##password-auth
        if len(self.password_faillock_preauth.get_line()) == 0:
            self.password_faillock_preauth.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH, LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN)

        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        if len(self.password_faillock_authfail.get_line()) == 0:
            self.password_faillock_authfail.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL, LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN)

        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        if len(self.password_faillock_authsucc.get_line()) == 0:
            self.password_faillock_authsucc.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC, LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN)

        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", args[LOGIN_LOCK_CONF_KEY_FAILURES], "=")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME], "=")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK], "")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME], "=")

        return (True, '')