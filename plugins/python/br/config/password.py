#--coding:utf8 --

import json
import br.configuration
import pwd
import spwd

PASSWORD_EXPIRED_CONF_PATH = "/etc/login.defs"
PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS = "PASS_MAX_DAYS"
PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS = "PASS_MIN_DAYS"
# PASSWORD_EXPIRED_CONF_KEY_MIN_LEN  = "PASS_MIN_LEN"
PASSWORD_EXPIRED_CONF_KEY_WARN_AGE = "PASS_WARN_AGE"

#PASSWORD_COMPLEXTIY_CONF_PATH  = "/etc/security/pwquality.conf.d/90-br-config.conf"
PASSWORD_COMPLEXTIY_SYSTEM_CONF_PATH  = "/etc/pam.d/system-auth"
PASSWORD_COMPLEXTIY_PASSWORD_CONF_PATH = "/etc/pam.d/password-auth"
PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY = "password    requisite                                    pam_pwquality.so try_first_pass local_users_only"
PASSWORD_COMPLEXITY_CONF_KEY_MINLEN  = "minlen"
PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL = "ucredit"
PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES = "lcredit"
PASSWORD_COMPLEXITY_CONF_KEY_NUMBER  =  "dcredit"
PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL = "ocredit"
PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS = "minclass"
PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION = "maxsequence"
PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK = "usercheck"

PASSWORD_COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN = "password\\s+sufficient\\s+pam_unix.so"

# 账户设置过期时间

# 不修改三权用户和root的账户过期时间 , 排除sftpuser
THREE_RIGHTS_USERS = ("sysadm","secadm","audadm","root","sftpuser")

PASSWORD_EXPIRED_ACCOUNTS_KEY = "accounts"
PASSWORD_EXPIRED_ACCOUNTS_EXPIRATION_KEY = "accounts_expiration"

# 获取当前时间
CURRENT_TIME_CMD = "date -d \"`date '+%Y-%m-%d'`\" +%s"

class PasswordExpired:
    def __init__(self):
        self.conf = br.configuration.KV(PASSWORD_EXPIRED_CONF_PATH)
        self.curtime =  br.utils.subprocess_has_output(CURRENT_TIME_CMD)
        # br.log.debug(1111)

    def get(self):
        retdata = dict()
        retdata[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS))
        retdata[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS))
        # retdata[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN]  = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN))
        retdata[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE] = int(self.conf.get_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE))
        # br.log.debug(22222)
        for pwdent in pwd.getpwall():
            cmd = 'chage -l {0} |grep 帐户过期时间 |grep 从不'.format(pwdent.pw_name)
            if THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                continue
            else:
                if len(br.utils.subprocess_has_output(cmd))  == 0:
                    retdata[PASSWORD_EXPIRED_ACCOUNTS_EXPIRATION_KEY]  = True
                    break
                else:
                    retdata[PASSWORD_EXPIRED_ACCOUNTS_EXPIRATION_KEY]  = False

        retdata[PASSWORD_EXPIRED_ACCOUNTS_KEY] = 90
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS, args[PASSWORD_EXPIRED_CONF_KEY_MAX_DAYS])
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS, args[PASSWORD_EXPIRED_CONF_KEY_MIN_DAYS])
        # self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_MIN_LEN, args[PASSWORD_EXPIRED_CONF_KEY_MIN_LEN])
        self.conf.set_value(PASSWORD_EXPIRED_CONF_KEY_WARN_AGE, args[PASSWORD_EXPIRED_CONF_KEY_WARN_AGE])
        
        # +86399s，表示到当天的23:59:59过期
        set_expiration_time = 'date -d @{0} "+%Y-%m-%d"'.format(args[PASSWORD_EXPIRED_ACCOUNTS_KEY] * 86400 + int(self.curtime) + 86399)
        expiration_time = br.utils.subprocess_has_output(set_expiration_time)

        cmd_sums = ""
        for pwdent in pwd.getpwall():
            if THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                continue
            else:
                if args[PASSWORD_EXPIRED_ACCOUNTS_EXPIRATION_KEY] == True:
                    set_acc_expired_cmd = 'chage -E {0} {1}'.format(expiration_time,pwdent.pw_name)
                    # br.log.debug(args[PASSWORD_EXPIRED_ACCOUNTS_KEY] * 86400)
                    cmd_sums = cmd_sums + set_acc_expired_cmd + ";"
                    # br.utils.subprocess_not_output(set_acc_expired_cmd)
                else:
                    set_acc_expired_cmd = 'chage -E -1 {0}'.format(pwdent.pw_name)
                    cmd_sums = cmd_sums + set_acc_expired_cmd + ";"
                    # br.utils.subprocess_not_output(set_acc_expired_cmd)
        br.utils.subprocess_not_output(cmd_sums)

        return (True, '')

class PasswordComplexity:
    def __init__(self):
        self.system_conf = br.configuration.PAM(PASSWORD_COMPLEXTIY_SYSTEM_CONF_PATH, "password\\s+requisite\\s+pam_pwquality.so")
        self.password_conf = br.configuration.PAM(PASSWORD_COMPLEXTIY_PASSWORD_CONF_PATH, "password\\s+requisite\\s+pam_pwquality.so")

    def get(self):
        retdata = dict()
        minlen_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_MINLEN, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_MINLEN] = int(minlen_value)

        capital_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL] = -(int(capital_value))

        minuscules_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES] = -(int(minuscules_value))

        number_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_NUMBER, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_NUMBER] = -(int(number_value))

        special_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL] = -(int(special_value))

        minclass_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS] = int(minclass_value)

        succession_value = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION] = int(succession_value)

        user_check = self.system_conf.get_value(PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK, '=')
        retdata[PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK] =  user_check == '0'

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if len(self.system_conf.get_line()) == 0:
                self.system_conf.set_line(PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY, PASSWORD_COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN)

        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINLEN, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_MINLEN], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_NUMBER, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_NUMBER], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION], '=')
        self.system_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK, '=', '0' if args[PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK] else '1', '=')

        if len(self.password_conf.get_line()) == 0:
                self.password_conf.set_line(PASSWORD_COMPLEXTIY_CONF_KEY_PWQUALITY, PASSWORD_COMPLEXITY_CONF_NEXT_MATCH_LINE_PATTERN)

        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINLEN, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_MINLEN], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_CAPITAL], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_MINUSCULES], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_NUMBER, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_NUMBER], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL, '=', -args[PASSWORD_COMPLEXITY_CONF_KEY_SPECIAL], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_MINCLASS], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION, '=', args[PASSWORD_COMPLEXITY_CONF_KEY_SUCCESSION], '=')
        self.password_conf.set_value(PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK, '=', '0' if args[PASSWORD_COMPLEXITY_CONF_KEY_USER_CHECK] else '1', '=')

        return (True, '')