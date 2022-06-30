#--coding:utf8 --

import json
import ssr.configuration
import ssr.systemd
import ssr.log

SSHD_CONF_PATH = "/etc/ssh/sshd_config"

# 允许root进行ssh远程登陆
ROOT_LOGIN_ARG_ENABLED = "enabled"
PUBKEY_AUTH_ARG_ENABLED = "enabled"
WEAK_ENCRYPT_ARG_ENABLED = "enabled"
BANNER_INFO_ARG_ENABLED = "enabled"

BANNER_INFO_KEY = "Banner"

# 会话登录后超过指定时间未操作则自动退出
PROFILE_CLIENT_TMOUT = "ClientAliveInterval"
PROFILE_CLIENT_COUNT = "ClientAliveCountMax"

#用于匹配TMOUT进行修改
PROFILE_TMOUT = "TMOUT"
PROFILE_TMOUT_RXPORT = "export TMOUT"

DEFAULT_CIPHERS = ("aes128-ctr", "aes192-ctr", "aes256-ctr", "aes128-cbc", "3des-cbc")
WEAK_CIPHERS = ("arcfour", "arcfour128", "arcfour256")

# sshd服务限制

SSHD_CONF_PROTOCOL_KEY = "protocol"
SSHD_CONF_PERMIT_EMPTY_KEY = "permitEmpty"
SSHD_CONF_PORT_KEY = "port"
SSHD_CONF_PAM_KEY = "pam"

# sshd服务限制匹配字段
SSHD_CONF_PROTOCOL = "Protocol"
SSHD_CONF_PERMIT_EMPTY = "PermitEmptyPasswords"
SSHD_CONF_PORT = "Port"
SSHD_CONF_PAM = "UsePAM"

SET_SFTP_USER_CMD = "useradd sftpuser"
SET_SFTP_USER_CONFIG = "Match User sftpuser\nChrootDirectory /home\nX11Forwarding no\nAllowTcpForwarding no\nForceCommand internal-sftp"

class SSHD:
    def __init__(self):
        self.conf = ssr.configuration.KV(SSHD_CONF_PATH, join_string=" ")
        self.service = ssr.systemd.Proxy("sshd")


class RootLogin(SSHD):
    def get(self):
        retdata = dict()
        retdata[ROOT_LOGIN_ARG_ENABLED] = (self.conf.get_value("PermitRootLogin") == "yes")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        self.conf.set_value("PermitRootLogin", "yes" if args[ROOT_LOGIN_ARG_ENABLED] else "no")
        # 重启服务生效
        self.service.reload()
        return (True, '')


class PubkeyAuth(SSHD):
    def get(self):
        retdata = dict()
        retdata[PUBKEY_AUTH_ARG_ENABLED] = (not (self.conf.get_value("PubkeyAuthentication") == "no"))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        self.conf.set_value("PubkeyAuthentication", "yes" if args[PUBKEY_AUTH_ARG_ENABLED] else "no")
        # 重启服务生效
        self.service.reload()
        return (True, '')

class WeakEncryption(SSHD):
    def get(self):
        retdata = dict()
        ciphers = self.conf.get_value("Ciphers").split(",")

        if ciphers.__contains__("arcfour") or ciphers.__contains__("arcfour128") or ciphers.__contains__("arcfour256"):
            retdata[WEAK_ENCRYPT_ARG_ENABLED] = True
        else:
            retdata[WEAK_ENCRYPT_ARG_ENABLED] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        ciphers = self.conf.get_value("Ciphers").split(",")
        # 过滤空元素
        ciphers = filter(lambda x: x, ciphers)

        # 这里只处理不允许弱加密算法的情况。如果是允许弱加密算法，这里也不会把弱加密算法添加进去，因为这会导致新版本sshd无法启动
        if not args[WEAK_ENCRYPT_ARG_ENABLED]:
            # 如果未空说明是默认配置，新版本中默认配置是不支持若加密算法，因此只考虑非默认配置的情况
            if (len(ciphers) > 0):
                ciphers = [cipher for cipher in ciphers if cipher.find("arcfour") == -1]
            # 如果列表为空，则改为默认值
            if len(ciphers) == 0:
                self.conf.del_record("Ciphers")
            else:
                self.conf.set_value("Ciphers", ','.join(ciphers))
            # 重启服务生效
            self.service.reload()
        return (True, '')

class BannerInfo(SSHD):
    def get(self):
        retdata = dict()
        retdata[BANNER_INFO_KEY] = self.conf.get_value("Banner")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        #self.conf.set_value("Banner", "/etc/issue.net" if args[ROOT_LOGIN_ARG_ENABLED] else "none")
        #只对开启后关闭做处理
        # if args[ROOT_LOGIN_ARG_ENABLED]:
        self.conf.set_value("Banner", args[BANNER_INFO_KEY])
        # 重启服务生效
        self.service.reload()
        return (True, '')

class SessionTimeout(SSHD):
    def get(self):
        retdata = dict()
        tmout = self.conf.get_value(PROFILE_CLIENT_TMOUT)
        count = self.conf.get_value(PROFILE_CLIENT_COUNT)

        retdata[PROFILE_CLIENT_TMOUT] = int(tmout)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        self.conf_profile = ssr.configuration.KV("/etc/profile", "=", "=")
        self.conf_bashrc = ssr.configuration.KV("/etc/bashrc","=", "=")

        if (args[PROFILE_CLIENT_TMOUT] <= 0):
            self.conf.del_record(PROFILE_CLIENT_TMOUT)
            self.conf.del_record(PROFILE_CLIENT_COUNT)
            # 如果/etc/profile /etc/bashrc 中有TMOUT的值，则进行修改
            self.conf_profile.set_all_value(PROFILE_TMOUT,"")
            self.conf_profile.set_all_value(PROFILE_TMOUT_RXPORT,"")
            self.conf_bashrc.set_all_value(PROFILE_TMOUT,"")
            self.conf_bashrc.set_all_value(PROFILE_TMOUT_RXPORT,"")
        else:
            # 如果/etc/profile /etc/bashrc 中有TMOUT的值，则进行修改
            if self.conf_profile.get_value(PROFILE_TMOUT):
                self.conf_profile.set_all_value(PROFILE_TMOUT,args[PROFILE_CLIENT_TMOUT])
            if self.conf_profile.get_value(PROFILE_TMOUT_RXPORT):
                self.conf_profile.set_all_value(PROFILE_TMOUT_RXPORT,args[PROFILE_CLIENT_TMOUT])
            if self.conf_bashrc.get_value(PROFILE_TMOUT):
                self.conf_bashrc.set_all_value(PROFILE_TMOUT,args[PROFILE_CLIENT_TMOUT])
            if self.conf_bashrc.get_value(PROFILE_TMOUT_RXPORT):
                self.conf_bashrc.set_all_value(PROFILE_TMOUT_RXPORT,args[PROFILE_CLIENT_TMOUT])
            #self.conf_bashrc.set_all_value(PROFILE_TMOUT,"")
            #self.conf_bashrc.set_all_value(PROFILE_TMOUT_RXPORT,"")
            if self.conf.get_value(PROFILE_CLIENT_TMOUT) != str(args[PROFILE_CLIENT_TMOUT]):
                self.conf.set_value(PROFILE_CLIENT_TMOUT, args[PROFILE_CLIENT_TMOUT])
            if self.conf.get_value(PROFILE_CLIENT_COUNT) != 0:
                self.conf.set_value(PROFILE_CLIENT_COUNT, 0)

        # 重启服务生效
        self.service.reload()
        return (True, '')

class SshdService(SSHD):
    def get(self):
        retdata = dict()
        retdata[SSHD_CONF_PROTOCOL_KEY] = (self.conf.get_value(SSHD_CONF_PROTOCOL) == "2")
        retdata[SSHD_CONF_PERMIT_EMPTY_KEY] = (not (self.conf.get_value(SSHD_CONF_PERMIT_EMPTY) == "no"))
        retdata[SSHD_CONF_PORT_KEY] = (self.conf.get_value(SSHD_CONF_PORT) == "1022")
        retdata[SSHD_CONF_PAM_KEY] = (not (self.conf.get_value(SSHD_CONF_PAM) == "no"))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        args = json.loads(args_json)
        self.conf.set_value(SSHD_CONF_PROTOCOL, "2" if args[SSHD_CONF_PROTOCOL_KEY] else "")
        self.conf.set_value(SSHD_CONF_PERMIT_EMPTY, "yes" if args[SSHD_CONF_PERMIT_EMPTY_KEY] else "no")
        self.conf.set_value(SSHD_CONF_PORT, "1022" if args[SSHD_CONF_PORT_KEY] else "")
        self.conf.set_value(SSHD_CONF_PAM, "yes" if args[SSHD_CONF_PAM_KEY] else "no")

        # 重启服务生效
        self.service.reload()
        return (True, '')

class SftpUser(SSHD):
    def user_exist(self,username):
        cmd = 'if id -u {0} >/dev/null 2>&1 ; then echo exist; fi'.format(username)
        if len(ssr.utils.subprocess_has_output(cmd)) == 0:
            return False
        else:
            return True

    def get(self):
        retdata = dict()
        retdata["enabled"] = self.user_exist("sftpuser")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False,'sshd.services is not running! \t\t')
        self.conf_table = ssr.configuration.Table(SSHD_CONF_PATH, ",\\s+")
        args = json.loads(args_json)
        if args["enabled"]:
            if not self.user_exist("sftpuser"):
                ssr.utils.subprocess_not_output(SET_SFTP_USER_CMD)
            self.conf_table.set_value("Match User",SET_SFTP_USER_CONFIG)
        else:
            if self.user_exist("sftpuser"):
                rm_cmd = "userdel -r sftpuser"
                ssr.utils.subprocess_not_output(rm_cmd)
            self.conf_table.del_record("Match User")

        # 重启服务生效
        self.service.reload()
        return (True, '')