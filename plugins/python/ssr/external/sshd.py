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

DEFAULT_CIPHERS = ("aes128-ctr", "aes192-ctr", "aes256-ctr", "aes128-cbc", "3des-cbc")
WEAK_CIPHERS = ("arcfour", "arcfour128", "arcfour256")


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
        args = json.loads(args_json)
        self.conf.set_value("PermitRootLogin", "yes" if args[ROOT_LOGIN_ARG_ENABLED] else "no")
        # 重启服务生效
        self.service.restart()
        return (True, '')


class PubkeyAuth(SSHD):
    def get(self):
        retdata = dict()
        retdata[PUBKEY_AUTH_ARG_ENABLED] = (not (self.conf.get_value("PubkeyAuthentication") == "no"))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value("PubkeyAuthentication", "yes" if args[PUBKEY_AUTH_ARG_ENABLED] else "no")
        # 重启服务生效
        self.service.restart()
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
            self.service.restart()
        return (True, '')

class BannerInfo(SSHD):
    def get(self):
        retdata = dict()
        retdata[BANNER_INFO_ARG_ENABLED] = (self.conf.get_value("Banner") == "/etc/issue.net")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        self.conf.set_value("Banner", "/etc/issue.net" if args[ROOT_LOGIN_ARG_ENABLED] else "none")
        # 重启服务生效
        self.service.restart()
        return (True, '')