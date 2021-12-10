#--coding:utf8 --

import json
import ssr.configuration

PASSWORD_EXPIRED_CONF_PATH = "/etc/login.defs"
PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS = "PASS_MAX_DAYS"
PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS = "PASS_MIN_DAYS"
PASSWORD_EXPIRED_CONF_KEY_MIN_LEN  = "PASS_MIN_LEN"
PASSWORD_EXPIRED_CONF_KEY_WARN_AGE = "PASS_WARN_AGE"

PASSWORD_COMPLEXTIY_CONF_PATH  = "/etc/security/pwquality.conf"
#PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY = "password    requisite                                    pam_pwquality.so try_first_pass local_users_only"
PASSWORD_COMPLEXITY_CONF_KEY_MINLEN  = "minlen"
PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL = "ucredit"
PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES = "lcredit"
PASSWORD_COMPLEXITY_CONF_KEY_NUMBER  =  "dcredit"
PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL = "ocredit"
PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION = "maxsequence"
PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK = "reject_username"

class PasswordExpired:
    def __init__(self):
        self.conf = ssr.configuration.KV(PASSWORD_EXPIRED_CONF_PATH)

    def get(self):
        retdata = dict()
        retdata[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS))
        retdata[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS))
        retdata[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN]  = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN))
        retdata[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS, args[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS])
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS, args[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS])
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN, args[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN])
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE, args[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE])

        return (True, '')

class PasswordComplexity:
    def __init__(self):
        self.conf = ssr.configuration.KV(PASSWORD_COMPLEXTIY_CONF_PATH, "=", "=")

    def get(self):
        retdata = dict()
        minlen_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_MINLEN)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_MINLEN] = int(minlen_value)

        capital_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL] = int(capital_value)

        minuscules_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES] = int(minuscules_value)

        number_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_NUMBER)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_NUMBER] = int(number_value)

        special_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL] = int(special_value)

        succession_value = self.conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION)
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION] = int(succession_value)

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINLEN, args[PASSWORD_COMPLEXITY_CONF_KEY_MINLEN])
        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL, args[PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL])
        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES, args[PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES])
        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_NUMBER, args[PASSWORD_COMPLEXITY_CONF_KEY_NUMBER])
        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL, args[PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL])
        self.conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION, args[PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION])

        return (True, '')