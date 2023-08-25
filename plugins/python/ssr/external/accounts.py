# -*- coding: utf-8 -*-

try:
    import configparser
except:
    import ConfigParser as configparser

import json
import os
import pwd
import spwd
import ssr.vars
import ssr.utils

MINIMUM_UID = 1000
BUILTIN_IGNORE_USRES = ("bin", "daemon", "adm", "lp", "sync", "shutdown", "halt", "mail", "news", "uucp", "nobody", "postgres", "pvm", "rpm",
                        "nfsnobody", "pcap", "mysql", "ftp", "games", "man", "at", "gdm", "gnome-initial-setup")
BUILTIN_PERMISSION_USERS = ("root")

# 移除三权和sftpuser账户的空密码检测
THREE_RIGHTS_USERS = ("sysadm", "secadm", "audadm", "sftpuser")

ACCOUNTS_INI_FILEPATH = ssr.vars.SSR_PLUGIN_PYTHON_ROOT_DIR + \
    "/ssr/external/accounts.ini"
ACCOUNTS_GROUP_LOGIN_LIMIT = "LoginLimit"
# LPK: Accounts LoginLimit Key
ALK_MODE_PERMISSION_USERS = "PermissionUsers"

# 开启无关账号设置为不可登陆(默认保留root账号、三权账号、普通账号)
LOGIN_LIMIT_ARG_ENABLED = "enabled"
# 用户自定义允许登陆账号列表
LOGIN_LIMIT_ARG_PERMISSION_USERS = "permission-users"

# 禁止存在空密码账号
NULL_PASSWORD_ARG_ENABLED = "enabled"

# 多余用户
ACCOUNTS_GROUP_SURPLUS = "SurplusUser"
# 用户自定义删除用户
SURPLUS_DELETE_USERS = "delete-users"
# LPK: Accounts LoginLimit Key
ALK_MODE_DELETE_USERS = "DeleteUsers"
# 默认删除用户
DEAFULT_DELETE_USERS = ("lp", "games", "operator", "adm")

SURPLUS_DELETE_ENABLED = "enabled"

# GET_USER_NAME_CMD = "eval getent passwd {$(awk '/^UID_MIN/ {print $2}' /etc/login.defs)..$(awk '/^UID_MAX/ {print $2}' /etc/login.defs)} | cut -d: -f1"
GET_MINIMUM_UID = "awk '/^UID_MIN/ {print $2}' /etc/login.defs"
GET_MAXIMUM_UID = "awk '/^UID_MAX/ {print $2}' /etc/login.defs"


class Accounts:
    def is_nologin_shell(self, shell):
        basename = os.path.basename(shell)
        if len(shell) == 0 or basename == "nologin" or basename == "false":
            return True
        return False

    def is_human(self, uid, username, shell):
        if self.is_nologin_shell(shell):
            return False

        ssr.log.debug("name = ", username, "uid = ", uid)
        MINIMUM_UID = int(ssr.utils.subprocess_has_output(GET_MINIMUM_UID))
        MAXIMUM_UID = int(ssr.utils.subprocess_has_output(GET_MAXIMUM_UID))

        return (uid < MINIMUM_UID) or (uid > MAXIMUM_UID)

    def is_null_pw_human(self, uid, username, shell):
        if self.is_nologin_shell(shell):
            return False
        if BUILTIN_IGNORE_USRES.__contains__(username):
            return False
        return uid >= MINIMUM_UID

    def is_null_password(self, username):
        spwdent = spwd.getspnam(username)
        # 兼容python2和python3
        try:
            return (spwdent.sp_pwd == "" or spwdent.sp_pwd == "!!" or spwdent.sp_pwd == "!")
        except:
            return (spwdent.sp_pwdp == "" or spwdent.sp_pwdp == "!!" or spwdent.sp_pwdp == "!")

    # def get_user_name(self, permission_users):
    #     output = ssr.utils.subprocess_has_output(GET_USER_NAME_CMD)
    #     permission_users += output.encode().split('\n')
    #     ssr.log.debug(list(permission_users))


class LoginLimit(Accounts):
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(ACCOUNTS_INI_FILEPATH)

    def get(self):
        retdata = dict()
        retdata[LOGIN_LIMIT_ARG_ENABLED] = True
        retdata[LOGIN_LIMIT_ARG_PERMISSION_USERS] = self.conf.get(
            ACCOUNTS_GROUP_LOGIN_LIMIT, ALK_MODE_PERMISSION_USERS)
        permission_users = retdata[LOGIN_LIMIT_ARG_PERMISSION_USERS].split(";")

        for pwdent in pwd.getpwall():
            if (not self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell) or BUILTIN_PERMISSION_USERS.__contains__(pwdent.pw_name)
                    or permission_users.__contains__(pwdent.pw_name)):
                continue
            if not self.is_nologin_shell(pwdent.pw_shell):
                retdata[LOGIN_LIMIT_ARG_ENABLED] = False
                break

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.set(ACCOUNTS_GROUP_LOGIN_LIMIT, ALK_MODE_PERMISSION_USERS,
                      args[LOGIN_LIMIT_ARG_PERMISSION_USERS])
        self.conf.write(open(ACCOUNTS_INI_FILEPATH, 'wb'))
        permission_users = args[LOGIN_LIMIT_ARG_PERMISSION_USERS].split(";")

        if args[LOGIN_LIMIT_ARG_ENABLED]:
            for pwdent in pwd.getpwall():
                if ((not self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell)) or BUILTIN_PERMISSION_USERS.__contains__(pwdent.pw_name)
                        or permission_users.__contains__(pwdent.pw_name)):
                    ssr.log.debug(str(pwdent.pw_name))
                    continue
                if not self.is_nologin_shell(pwdent.pw_shell):
                    ssr.utils.subprocess_not_output(
                        "usermod -s /sbin/nologin {0}".format(pwdent.pw_name))
        # 过检需求，这个名单直接设置为可登录
        for permission_user in permission_users:
            if permission_user != "":
                ssr.utils.subprocess_not_output(
                    "usermod -s /bin/bash {0}".format(permission_user))

        return (True, '')


class NullPassword(Accounts):
    def get(self):
        retdata = dict()
        retdata[NULL_PASSWORD_ARG_ENABLED] = True

        for pwdent in pwd.getpwall():
            if (not self.is_null_pw_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell)) or THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                # ssr.log.debug("pwdent.pw_name = ", pwdent.pw_name, "is_human = ", self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell))
                continue
            if self.is_null_password(pwdent.pw_name):
                retdata[NULL_PASSWORD_ARG_ENABLED] = False
                break

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[NULL_PASSWORD_ARG_ENABLED]:
            for pwdent in pwd.getpwall():
                if (not self.is_null_pw_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell)) or THREE_RIGHTS_USERS.__contains__(pwdent.pw_name):
                    ssr.log.debug("pop  pwdent.pw_name = ",
                                  pwdent.pw_name, "pw_uid = ", pwdent.pw_uid)
                    continue
                if self.is_null_password(pwdent.pw_name) and pwdent.pw_uid != 0:
                    ssr.log.debug("del  pwdent.pw_name = ",
                                  pwdent.pw_name, "pw_uid = ", pwdent.pw_uid)
                    ssr.utils.subprocess_not_output(
                        "userdel -r {0} &> /dev/null ||: ".format(pwdent.pw_name))

        return (True, '')


class SurplusUser():
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(ACCOUNTS_INI_FILEPATH)

    def get(self):
        retdata = dict()
        retdata[SURPLUS_DELETE_ENABLED] = True
        retdata[SURPLUS_DELETE_USERS] = self.conf.get(
            ACCOUNTS_GROUP_SURPLUS, ALK_MODE_DELETE_USERS)
        delete_users = retdata[SURPLUS_DELETE_USERS].split(";")

        for pwdent in pwd.getpwall():
            if (DEAFULT_DELETE_USERS.__contains__(pwdent.pw_name)
                    or delete_users.__contains__(pwdent.pw_name)):
                retdata[SURPLUS_DELETE_ENABLED] = False
                break
            else:
                continue

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.set(ACCOUNTS_GROUP_SURPLUS,
                      ALK_MODE_DELETE_USERS, args[SURPLUS_DELETE_USERS])
        self.conf.write(open(ACCOUNTS_INI_FILEPATH, 'wb'))
        delete_users = args[SURPLUS_DELETE_USERS].split(";")

        if args[SURPLUS_DELETE_ENABLED]:
            for pwdent in pwd.getpwall():
                if (DEAFULT_DELETE_USERS.__contains__(pwdent.pw_name)
                        or delete_users.__contains__(pwdent.pw_name)):
                    ssr.log.debug(str(pwdent.pw_name))
                    if pwdent.pw_uid != 0:
                        ssr.utils.subprocess_not_output(
                            "userdel -r {0}  &> /dev/null ||: ".format(pwdent.pw_name))
                        ssr.utils.subprocess_not_output(
                            "groupdel {0}  &> /dev/null ||: ".format(pwdent.pw_name))
                else:
                    continue

        return (True, '')
