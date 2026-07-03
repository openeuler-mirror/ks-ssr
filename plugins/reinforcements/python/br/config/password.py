# -*- coding: utf-8 -*-

import json
import br.configuration
import pwd
import spwd

EXPIRED_CONF_PATH = "/etc/login.defs"
EXPIRED_CONF_KEY_MAX_DAYS = "PASS_MAX_DAYS"
EXPIRED_CONF_KEY_MIN_DAYS = "PASS_MIN_DAYS"
EXPIRED_CONF_KEY_WARN_AGE = "PASS_WARN_AGE"

COMPLEXTIY_CONF_PATH = "/etc/security/pwquality.conf"
COMPLEXTIY_SYSTEM_CONF_PATH = "/etc/pam.d/system-auth"
AUTH_COMPLEXTIY_CONF_PATH = "/etc/pam.d/password-auth"
COMPLEXTIY_CONF_KEY_PWQUALITY = "password    requisite                                    pam_pwquality.so try_first_pass local_users_only"
COMPLEXITY_CONF_KEY_MINLEN = "minlen"
COMPLEXITY_CONF_KEY_CAPITAL = "ucredit"
COMPLEXITY_CONF_KEY_MINUSCULES = "lcredit"
COMPLEXITY_CONF_KEY_NUMBER = "dcredit"
COMPLEXITY_CONF_KEY_SPECIAL = "ocredit"
COMPLEXITY_CONF_KEY_MINCLASS = "minclass"
COMPLEXITY_CONF_KEY_SUCCESSION = "maxsequence"
COMPLEXITY_CONF_KEY_USER_CHECK = "usercheck"
COMPLEXITY_CONF_KEY_DICT_CHECK = "dictcheck"

COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN = "password\\s+sufficient\\s+pam_unix.so"

# 账户设置过期时间

# 不修改三权用户和root的账户过期时间 , 排除sftpuser
THREE_RIGHTS_USERS = ("sysadm", "secadm", "audadm", "root", "sftpuser")

EXPIRED_ACCOUNTS_KEY = "accounts"
EXPIRED_ACCOUNTS_EXPIRATION_KEY = "accounts_expiration"

# 获取当前时间
CURRENT_TIME_CMD = "date -d \"`date '+%Y-%m-%d'`\" +%s"


class PasswordExpired:
    def __init__(self):
        self.conf = br.configuration.KV(EXPIRED_CONF_PATH)
        self.curtime = br.utils.subprocess_has_output(CURRENT_TIME_CMD)

    def get(self):
        retdata = dict()
        max_days = self.conf.get_value(EXPIRED_CONF_KEY_MAX_DAYS)
        min_days = self.conf.get_value(EXPIRED_CONF_KEY_MIN_DAYS)
        warn_age = self.conf.get_value(EXPIRED_CONF_KEY_WARN_AGE)

        retdata[EXPIRED_CONF_KEY_MAX_DAYS] = "" if not max_days else int(max_days)
        retdata[EXPIRED_CONF_KEY_MIN_DAYS] = "" if not min_days else int(min_days)
        retdata[EXPIRED_CONF_KEY_WARN_AGE] = "" if not warn_age else int(warn_age)
        for pwdent in pwd.getpwall():
            cmd = 'chage -l {0} |grep 帐户过期时间 |grep 从不'.format(pwdent.pw_name)
            if THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                continue
            else:
                if len(br.utils.subprocess_has_output(cmd)) == 0:
                    retdata[EXPIRED_ACCOUNTS_EXPIRATION_KEY] = True
                    break
                else:
                    retdata[EXPIRED_ACCOUNTS_EXPIRATION_KEY] = False

        retdata[EXPIRED_ACCOUNTS_KEY] = 90
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value(EXPIRED_CONF_KEY_MAX_DAYS,
                            args[EXPIRED_CONF_KEY_MAX_DAYS])
        self.conf.set_value(EXPIRED_CONF_KEY_MIN_DAYS,
                            args[EXPIRED_CONF_KEY_MIN_DAYS])
        # self.conf.set_value(EXPIRED_CONF_KEY_MIN_LEN, args[EXPIRED_CONF_KEY_MIN_LEN])
        self.conf.set_value(EXPIRED_CONF_KEY_WARN_AGE,
                            args[EXPIRED_CONF_KEY_WARN_AGE])

        # +86399s，表示到当天的23:59:59过期
        set_expiration_time = 'date -d @{0} "+%Y-%m-%d"'.format(
            args[EXPIRED_ACCOUNTS_KEY] * 86400 + int(self.curtime) + 86399)
        expiration_time = br.utils.subprocess_has_output(set_expiration_time)

        cmd_sums = ""
        for pwdent in pwd.getpwall():
            if THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                continue
            else:
                if args[EXPIRED_ACCOUNTS_EXPIRATION_KEY] == True:
                    set_acc_expired_cmd = 'chage -E {0} {1}'.format(
                        expiration_time, pwdent.pw_name)
                    # br.log.debug(args[EXPIRED_ACCOUNTS_KEY] * 86400)
                    cmd_sums = cmd_sums + set_acc_expired_cmd + ";"
                    # br.utils.subprocess_not_output(set_acc_expired_cmd)
                else:
                    set_acc_expired_cmd = 'chage -E -1 {0}'.format(
                        pwdent.pw_name)
                    cmd_sums = cmd_sums + set_acc_expired_cmd + ";"
                    # br.utils.subprocess_not_output(set_acc_expired_cmd)
        br.utils.subprocess_not_output(cmd_sums)

        return (True, '')


class PasswordComplexity:
    def __init__(self):
        self.system_conf = br.configuration.PAM(
            COMPLEXTIY_SYSTEM_CONF_PATH, "password\\s+requisite\\s+pam_pwquality.so")
        self.password_conf = br.configuration.PAM(
            AUTH_COMPLEXTIY_CONF_PATH, "password\\s+requisite\\s+pam_pwquality.so")
        self.conf = br.configuration.KV(
            COMPLEXTIY_CONF_PATH, "\\s*=\\s*", " = ")
    
    def set_system_auth(self, key_minlen, capital, lcredit, number, special, min_class, succession, user_check):
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_MINLEN, '=', "" if not str(key_minlen) else int(key_minlen), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_CAPITAL, '=', "" if not str(capital) else -int(capital), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_MINUSCULES, '=', "" if not str(lcredit) else -int(lcredit), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_NUMBER, '=', "" if not str(number) else -int(number), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_SPECIAL, '=', "" if not str(special) else -int(special), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_MINCLASS, '=', "" if not str(min_class) else int(min_class), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_SUCCESSION, '=', "" if not str(succession) else int(succession), '=')
        self.system_conf.set_value(COMPLEXITY_CONF_KEY_USER_CHECK, '=', "" if not str(user_check) else int(not user_check), '=')

    def set_pw_auth(self, key_minlen, capital, lcredit, number, special, min_class, succession, user_check):
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_MINLEN, '=', "" if not str(key_minlen) else int(key_minlen), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_CAPITAL, '=', "" if not str(capital) else -int(capital), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_MINUSCULES,'=', "" if not str(lcredit) else -int(lcredit), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_NUMBER, '=', "" if not str(number) else -int(number), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_SPECIAL, '=', "" if not str(special) else -int(special), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_MINCLASS, '=', "" if not str(min_class) else int(min_class), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_SUCCESSION, '=', "" if not str(succession) else int(succession), '=')
        self.password_conf.set_value(COMPLEXITY_CONF_KEY_USER_CHECK, '=', "" if not str(user_check) else int(not user_check), '=')

    def get(self):
        retdata = dict()
        minlen_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_MINLEN, '=')
        retdata[COMPLEXITY_CONF_KEY_MINLEN] = "" if not minlen_value else int(minlen_value)

        capital_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_CAPITAL, '=')
        retdata[COMPLEXITY_CONF_KEY_CAPITAL] = "" if not capital_value else -(int(capital_value))

        minuscules_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_MINUSCULES, '=')
        retdata[COMPLEXITY_CONF_KEY_MINUSCULES] = "" if not minuscules_value else -(int(minuscules_value))

        number_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_NUMBER, '=')
        retdata[COMPLEXITY_CONF_KEY_NUMBER] = "" if not number_value else -(int(number_value))

        special_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_SPECIAL, '=')
        retdata[COMPLEXITY_CONF_KEY_SPECIAL] = "" if not special_value else -(int(special_value))

        minclass_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_MINCLASS, '=')
        retdata[COMPLEXITY_CONF_KEY_MINCLASS] = "" if not minclass_value else int(minclass_value)

        succession_value = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_SUCCESSION, '=')
        retdata[COMPLEXITY_CONF_KEY_SUCCESSION] = "" if not succession_value else int(succession_value)

        user_check = self.system_conf.get_value(
            COMPLEXITY_CONF_KEY_USER_CHECK, '=')
        retdata[COMPLEXITY_CONF_KEY_USER_CHECK] = "" if not user_check else user_check == "0"

        dict_check = self.conf.get_value(
            COMPLEXITY_CONF_KEY_DICT_CHECK)
        retdata[COMPLEXITY_CONF_KEY_DICT_CHECK] = "" if not dict_check else dict_check != "0"

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if len(self.system_conf.get_line()) == 0:
            self.system_conf.set_line(
                COMPLEXTIY_CONF_KEY_PWQUALITY, COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN)
        self.set_system_auth(args[COMPLEXITY_CONF_KEY_MINLEN],
                            args[COMPLEXITY_CONF_KEY_CAPITAL],
                            args[COMPLEXITY_CONF_KEY_MINUSCULES],
                            args[COMPLEXITY_CONF_KEY_NUMBER],
                            args[COMPLEXITY_CONF_KEY_SPECIAL],
                            args[COMPLEXITY_CONF_KEY_MINCLASS],
                            args[COMPLEXITY_CONF_KEY_SUCCESSION],
                            args[COMPLEXITY_CONF_KEY_USER_CHECK])
        
        if len(self.password_conf.get_line()) == 0:
            self.password_conf.set_line(
                COMPLEXTIY_CONF_KEY_PWQUALITY, COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN)

        self.set_pw_auth(args[COMPLEXITY_CONF_KEY_MINLEN],
                        args[COMPLEXITY_CONF_KEY_CAPITAL],
                        args[COMPLEXITY_CONF_KEY_MINUSCULES],
                        args[COMPLEXITY_CONF_KEY_NUMBER],
                        args[COMPLEXITY_CONF_KEY_SPECIAL],
                        args[COMPLEXITY_CONF_KEY_MINCLASS],
                        args[COMPLEXITY_CONF_KEY_SUCCESSION],
                        args[COMPLEXITY_CONF_KEY_USER_CHECK])

        self.conf.set_all_value(COMPLEXITY_CONF_KEY_DICT_CHECK, "" if not str(args[COMPLEXITY_CONF_KEY_DICT_CHECK]) else int(args[COMPLEXITY_CONF_KEY_DICT_CHECK]))
        return (True, '')
