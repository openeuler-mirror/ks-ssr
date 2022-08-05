#--coding:utf8 --

import json
import os
import ssr.configuration
import ssr.systemd
# import ssr.sshd

HOSTS_ALLOW_CONF_PATH = "/etc/hosts.allow"
HOSTS_DENY_CONF_PATH = "/etc/hosts.deny"

REMOTE_LOGIN_ARG_ALLOW_HOSTS = "allow-hosts"
REMOTE_LOGIN_ARG_DENY_HOSTS = "deny-hosts"

SSHD_CONF_PATH = "/etc/ssh/sshd_config"

SSHD_DENYUSERS_KEY = "DenyUsers"
SSHD_ALLOWUSERS_KEY = "AllowUsers"

class Hosts:
    def __init__(self):
        self.allow_conf = ssr.configuration.KV(HOSTS_ALLOW_CONF_PATH, ":\\s*", ": ")
        self.deny_conf = ssr.configuration.KV(HOSTS_DENY_CONF_PATH, ":\\s*", ": ")
        self.conf = ssr.configuration.KV(SSHD_CONF_PATH, join_string=" ")
        self.service = ssr.systemd.Proxy("sshd")
        self.conf_allow = ssr.configuration.PAM(SSHD_CONF_PATH, SSHD_ALLOWUSERS_KEY)
        self.conf_deny = ssr.configuration.PAM(SSHD_CONF_PATH, SSHD_DENYUSERS_KEY)

    def check_sftp(self):
        if len(ssr.utils.subprocess_has_output("cat {0} |grep 'ForceCommand internal-sftp' ".format(SSHD_CONF_PATH))) != 0:
            # 添加
            next_line = ssr.utils.subprocess_has_output("cat {0} |grep 'Match User sftpuser' ".format(SSHD_CONF_PATH))
            if len(ssr.utils.subprocess_has_output("cat {0} |grep '{1}' ".format(SSHD_CONF_PATH,SSHD_ALLOWUSERS_KEY))) == 0:
                self.conf_allow.set_line(SSHD_ALLOWUSERS_KEY + " ssr", next_line)
            if len(ssr.utils.subprocess_has_output("cat {0} |grep '{1}' ".format(SSHD_CONF_PATH,SSHD_DENYUSERS_KEY))) == 0:
                self.conf_deny.set_line(SSHD_DENYUSERS_KEY + " ssr", next_line)
            # 注释
            # if len(self.conf.get_value(SSHD_ALLOWUSERS_KEY)) == 0:
            #     self.conf_allow.del_line()
            # if len(self.conf.get_value(SSHD_DENYUSERS_KEY)) == 0:
            #     self.conf_deny.del_line()

class RemoteLogin(Hosts):
    def get(self):
        retdata = dict()
        if os.path.exists(HOSTS_ALLOW_CONF_PATH):
            retdata[REMOTE_LOGIN_ARG_ALLOW_HOSTS] = str(self.allow_conf.get_value("sshd"))
            retdata[REMOTE_LOGIN_ARG_DENY_HOSTS] = str(self.deny_conf.get_value("sshd"))
        else:
            retdata[REMOTE_LOGIN_ARG_ALLOW_HOSTS] = str(self.conf.get_value(SSHD_ALLOWUSERS_KEY))
            retdata[REMOTE_LOGIN_ARG_DENY_HOSTS] = str(self.conf.get_value(SSHD_DENYUSERS_KEY))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if os.path.exists(HOSTS_ALLOW_CONF_PATH):
            if len(args[REMOTE_LOGIN_ARG_ALLOW_HOSTS]) > 0:
                self.allow_conf.set_value("sshd", args[REMOTE_LOGIN_ARG_ALLOW_HOSTS])
            else:
                self.allow_conf.del_record("sshd")

            if len(args[REMOTE_LOGIN_ARG_DENY_HOSTS]) > 0:
                self.deny_conf.set_value("sshd", args[REMOTE_LOGIN_ARG_DENY_HOSTS])
            else:
                self.deny_conf.del_record("sshd")
        else:
            # 不能加到sftp配置之后
            # 需特殊处理，使用PAM类型与KV类型相结合处理
            if len(args[REMOTE_LOGIN_ARG_ALLOW_HOSTS]) > 0:
                hosts = ""
                for host in args[REMOTE_LOGIN_ARG_ALLOW_HOSTS].split(","):
                    host = "*@" + host + "*"
                    hosts = hosts + " " + host 
                # 删除这一行再添加，KV类型无法获取以空格隔开的值
                ssr.utils.subprocess_not_output("sed -i '/{0}/d' {1}".format(SSHD_ALLOWUSERS_KEY, SSHD_CONF_PATH))
                self.check_sftp()
                self.conf.set_value(SSHD_ALLOWUSERS_KEY, hosts[1:])
            else:
                self.conf_allow.del_line()
            if len(args[REMOTE_LOGIN_ARG_DENY_HOSTS]) > 0:
                hosts = ""
                for host in args[REMOTE_LOGIN_ARG_DENY_HOSTS].split(","):
                    host = "*@" + host + "*"
                    hosts = hosts + " " + host 
                ssr.utils.subprocess_not_output("sed -i '/{0}/d' {1}".format(SSHD_DENYUSERS_KEY, SSHD_CONF_PATH))
                self.check_sftp()
                self.conf.set_value(SSHD_DENYUSERS_KEY, hosts[1:])
            else:
                self.conf_deny.del_line()
            self.service.reload()
        return (True, '')