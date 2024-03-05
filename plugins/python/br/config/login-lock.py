# -*- coding: utf-8 -*-

import json
import br.configuration

# 检查系统版本
# 对3.3和3.4做不同的处理
# 对于需要插入配置的情况处理,未开启登录锁定.

LIGHTDM_CONF_PATH = "/etc/pam.d/lightdm"
LOGIN_LOCK_SYSTEM_CONF_PATH = "/etc/pam.d/system-auth"
LOGIN_LOCK_PASSWORD_CONF_PATH = "/etc/pam.d/password-auth"
LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH = "auth        requisite                                    pam_faillock.so preauth audit"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL = "auth        [default=die]                                pam_faillock.so authfail audit"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC = "auth        sufficient                                   pam_faillock.so authsucc audit"
LOGIN_LOCK_CONF_KEY_FAILLOCK = "account     required                                     pam_faillock.so"

LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX = "auth\\s+requisite\\s+pam_faillock.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX = "auth\\s+\[default=die\]\\s+pam_faillock.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX = "auth\\s+sufficient\\s+pam_faillock.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_REGEX = "account\\s+required\\s+pam_faillock.so"

LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN = "auth\\s+required\\s+pam_faildelay.so"
LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN = "auth\\s+requisite\\s+pam_succeed_if.so"
LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN = "auth\\s+requisite\\s+pam_succeed_if.so"
LOGIN_LOCK_CONF_KEY_FAILLOCK_PATTERN = "account\\s+required\\s+pam_unix.so"

LIGHTDM_CONF_REGEX = "auth\\s+required\\s+pam_tally2.so"

LOGIN_LOCK_CONF_KEY_FAILURES = "deny"
LOGIN_LOCK_CONF_KEY_UNLOCK_TIME = "unlock_time"
LOGIN_LOCK_CONF_KEY_ROOT_LOCK = "even_deny_root"
LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME = "root_unlock_time"


class LoginLock:
    def __init__(self):
        self.system_faillock_preauth = br.configuration.PAM(
            LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX)
        self.system_faillock_authfail = br.configuration.PAM(
            LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX)
        self.system_faillock_authsucc = br.configuration.PAM(
            LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX)
        self.system_faillock_account = br.configuration.PAM(
            LOGIN_LOCK_SYSTEM_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_REGEX)

        self.password_faillock_preauth = br.configuration.PAM(
            LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH_REGEX)
        self.password_faillock_authfail = br.configuration.PAM(
            LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL_REGEX)
        self.password_faillock_authsucc = br.configuration.PAM(
            LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC_REGEX)
        self.password_faillock_account = br.configuration.PAM(
            LOGIN_LOCK_PASSWORD_CONF_PATH, LOGIN_LOCK_CONF_KEY_FAILLOCK_REGEX)

        self.lightdm_tally2 = br.configuration.PAM(
            LIGHTDM_CONF_PATH, LIGHTDM_CONF_REGEX)

    def set_deny(self, arg):
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")

        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_FAILURES, "=", arg, "=")

    def set_unlock_time(self, arg):
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")

        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=", arg, "=")
    
    def set_even_deny_root(self, arg):
        if arg:
            self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")
            self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")
            self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")

            self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")
            self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")
            self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "", arg, "")
        else:
            self.system_faillock_preauth.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
            self.system_faillock_authfail.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
            self.system_faillock_authsucc.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
        
            self.password_faillock_preauth.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
            self.password_faillock_authfail.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
            self.password_faillock_authsucc.del_record(LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")

    def set_root_unlock_time(self, arg):
        self.system_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")
        self.system_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")
        self.system_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")

        self.password_faillock_preauth.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")
        self.password_faillock_authfail.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")
        self.password_faillock_authsucc.set_value(LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=", arg, "=")

    def get(self):
        retdata = dict()
        deny_value = self.system_faillock_authfail.get_value(
            LOGIN_LOCK_CONF_KEY_FAILURES, "=")
        unlock_time = self.system_faillock_authfail.get_value(
            LOGIN_LOCK_CONF_KEY_UNLOCK_TIME, "=")
        even_deny_root = self.system_faillock_authfail.get_value(
            LOGIN_LOCK_CONF_KEY_ROOT_LOCK, "")
        root_unlock_time = self.system_faillock_authfail.get_value(
            LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME, "=")

        retdata[LOGIN_LOCK_CONF_KEY_FAILURES] = "" if not deny_value else int(deny_value)
        retdata[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME] = "" if not unlock_time else int(unlock_time)
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_LOCK] = even_deny_root != "false"
        retdata[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME] = "" if not root_unlock_time else int(root_unlock_time)

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        # Delete ligthdm tally2.so
        self.lightdm_tally2.del_line()

        # system-auth
        if len(self.system_faillock_preauth.get_line()) == 0:
            self.system_faillock_preauth.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH, LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN)
        if len(self.system_faillock_authfail.get_line()) == 0:
            self.system_faillock_authfail.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL, LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN)
        if len(self.system_faillock_authsucc.get_line()) == 0:
            self.system_faillock_authsucc.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC, LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN)
        if len(self.system_faillock_account.get_line()) == 0:
            self.system_faillock_account.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK, LOGIN_LOCK_CONF_KEY_FAILLOCK_PATTERN)

        # password-auth
        if len(self.password_faillock_preauth.get_line()) == 0:
            self.password_faillock_preauth.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_PREAUTH, LOGIN_LOCK_CONF_PREAUTH_NEXT_MATCH_LINE_PATTERN)
        if len(self.password_faillock_authfail.get_line()) == 0:
            self.password_faillock_authfail.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHFAIL, LOGIN_LOCK_CONF_AUTHFAIL_NEXT_MATCH_LINE_PATTERN)
        if len(self.password_faillock_authsucc.get_line()) == 0:
            self.password_faillock_authsucc.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK_AUTHSUCC, LOGIN_LOCK_CONF_AUTHSUCC_NEXT_MATCH_LINE_PATTERN)
        if len(self.password_faillock_account.get_line()) == 0:
            self.password_faillock_account.set_line(LOGIN_LOCK_CONF_KEY_FAILLOCK, LOGIN_LOCK_CONF_KEY_FAILLOCK_PATTERN)

        self.set_deny(args[LOGIN_LOCK_CONF_KEY_FAILURES])
        self.set_unlock_time(args[LOGIN_LOCK_CONF_KEY_UNLOCK_TIME])
        self.set_even_deny_root(args[LOGIN_LOCK_CONF_KEY_ROOT_LOCK])
        self.set_root_unlock_time(args[LOGIN_LOCK_CONF_KEY_ROOT_UNLOCK_TIME])

        return (True, '')
