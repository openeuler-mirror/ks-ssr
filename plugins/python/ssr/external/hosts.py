#--coding:utf8 --

import json
import ssr.configuration

HOSTS_ALLOW_CONF_PATH = "/etc/hosts.allow"
HOSTS_DENY_CONF_PATH = "/etc/hosts.deny"

REMOTE_LOGIN_ARG_ALLOW_HOSTS = "allow-hosts"
REMOTE_LOGIN_ARG_DENY_HOSTS = "deny-hosts"


class Hosts:
    def __init__(self):
        self.allow_conf = ssr.configuration.KV(HOSTS_ALLOW_CONF_PATH, ":\\s*", ": ")
        self.deny_conf = ssr.configuration.KV(HOSTS_DENY_CONF_PATH, ":\\s*", ": ")


class RemoteLogin(Hosts):
    def get(self):
        retdata = dict()
        retdata[REMOTE_LOGIN_ARG_ALLOW_HOSTS] = self.allow_conf.get_value("sshd")
        retdata[REMOTE_LOGIN_ARG_DENY_HOSTS] = self.deny_conf.get_value("sshd")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if len(args[REMOTE_LOGIN_ARG_ALLOW_HOSTS]) > 0:
            self.allow_conf.set_value("sshd", args[REMOTE_LOGIN_ARG_ALLOW_HOSTS])
        else:
            self.allow_conf.del_record("sshd")

        if len(args[REMOTE_LOGIN_ARG_DENY_HOSTS]) > 0:
            self.deny_conf.set_value("sshd", args[REMOTE_LOGIN_ARG_DENY_HOSTS])
        else:
            self.deny_conf.del_record("sshd")
        return (True, '')