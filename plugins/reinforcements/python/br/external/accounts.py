# -*- coding: utf-8 -*-

try:
    import configparser
except Exception:
    import ConfigParser as configparser

import json
import os
import pwd
import spwd
import br.vars
import br.utils

BUILTIN_IGNORE_USRES = (
    "bin",
    "daemon",
    "adm",
    "lp",
    "sync",
    "shutdown",
    "halt",
    "mail",
    "news",
    "uucp",
    "nobody",
    "postgres",
    "pvm",
    "rpm",
    "nfsnobody",
    "pcap",
    "mysql",
    "ftp",
    "games",
    "man",
    "at",
    "gdm",
    "gnome-initial-setup",
)
BUILTIN_PERMISSION_USERS = "root"

# 移除三权和sftpuser账户的空密码检测
THREE_RIGHTS_USERS = ("sysadm", "secadm", "audadm", "sftpuser")

ACCOUNTS_INI_FILEPATH = (
    br.vars.SSR_BR_PLUGIN_PYTHON_ROOT_DIR + "/br/external/accounts.ini"
)
ACCOUNTS_GROUP_LOGIN_LIMIT = "LoginLimit"
# LPK: Accounts LoginLimit Key
ALK_MODE_PERMISSION_USERS = "PermissionUsers"

# 开启无关账号设置为不可登陆(默认保留root账号、三权账号、普通账号)
LOGIN_LIMIT_ARG_ENABLED = "enabled"
# 用户自定义允许登陆账号列表
LOGIN_LIMIT_ARG_PERMISSION_USERS = "permission-users"

# 禁止存在空密码账号
NULL_ARG_ENABLED = "enabled"
NULL_PW_ACCOUNTS = "NullPWAccounts"

# 多余用户
ACCOUNTS_GROUP_SURPLUS = "SurplusUser"
# 用户自定义删除用户
SURPLUS_DELETE_USERS = "delete-users"
# LPK: Accounts LoginLimit Key
ALK_MODE_DELETE_USERS = "DeleteUsers"
# 默认删除用户
DEAFULT_DELETE_USERS = ("lp", "games", "operator", "adm")

SURPLUS_DELETE_ENABLED = "enabled"

GET_MINIMUM_UID = "awk '/^UID_MIN/ {print $2}' /etc/login.defs"
GET_MAXIMUM_UID = "awk '/^UID_MAX/ {print $2}' /etc/login.defs"

# 允许登录账号
ACCOUNTS_ALLOW_LOGIN = "AllowLogin"
# 不允许登录账号
ACCOUNTS_NOT_ALLOW_LOGIN = "NotAllowLogin"


class Accounts:
    def check_user_exists(self, username):
        try:
            # 使用pwd.getpwnam函数获取用户信息
            pwd.getpwnam(username)
            # 如果获取到用户信息，则用户存在
            return True
        except Exception as e:
            br.log.debug(e)
            return False

    def is_nologin_shell(self, shell):
        basename = os.path.basename(shell)
        if len(shell) == 0 or basename == "nologin" or basename == "false":
            return True
        return False

    def is_human(self, uid, username, shell):
        if self.is_nologin_shell(shell):
            return False

        br.log.debug("name = ", username, "uid = ", uid)
        MINIMUM_UID = int(br.utils.subprocess_has_output(GET_MINIMUM_UID))
        MAXIMUM_UID = int(br.utils.subprocess_has_output(GET_MAXIMUM_UID))
        return (uid >= MINIMUM_UID) and (uid <= MAXIMUM_UID)

    def is_null_pw_human(self, uid, username, shell):
        if self.is_nologin_shell(shell):
            return False
        if BUILTIN_IGNORE_USRES.__contains__(username):
            return False
        MINIMUM_UID = int(br.utils.subprocess_has_output(GET_MINIMUM_UID))
        return uid >= MINIMUM_UID

    def is_null_password(self, username):
        spwdent = spwd.getspnam(username)
        # 兼容python2和python3
        try:
            return (
                spwdent.sp_pwd == "" or spwdent.sp_pwd == "!!" or spwdent.sp_pwd == "!"
            )
        except Exception:
            return (
                spwdent.sp_pwdp == ""
                or spwdent.sp_pwdp == "!!"
                or spwdent.sp_pwdp == "!"
            )


class LoginLimit(Accounts):
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(ACCOUNTS_INI_FILEPATH)

    def set_permission_user(self, permission_users):
        # 过检需求，这个名单直接设置为可登录
        for permission_user in permission_users:
            if not permission_user:
                continue
            if not self.check_user_exists(permission_user):
                continue

            br.utils.subprocess_not_output(
                "usermod -s /bin/bash {0}".format(permission_user)
            )

    def get(self):
        retdata = dict()
        retdata[LOGIN_LIMIT_ARG_ENABLED] = True
        permission_value = self.conf.get(
            ACCOUNTS_GROUP_LOGIN_LIMIT, ALK_MODE_PERMISSION_USERS
        )
        retdata[LOGIN_LIMIT_ARG_PERMISSION_USERS] = (
            "" if not permission_value else permission_value
        )
        permission_users = retdata[LOGIN_LIMIT_ARG_PERMISSION_USERS].split(";")

        for pwdent in pwd.getpwall():
            # 可登录类型
            # 1.人为设置的账号（id大小超出系统定义的最小和最大用户ID范围）
            # 2.root
            # 3.界面设置的允许登录的账号（accounts.ini LoginLimit->PermissionUsers）
            if (
                not self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell)
                or BUILTIN_PERMISSION_USERS.__contains__(pwdent.pw_name)
                or permission_users.__contains__(pwdent.pw_name)
            ):
                continue
            # 其他账号不允许登录
            if not self.is_nologin_shell(pwdent.pw_shell):
                retdata[LOGIN_LIMIT_ARG_ENABLED] = False
                break

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if args[LOGIN_LIMIT_ARG_PERMISSION_USERS]:
            self.conf.set(
                ACCOUNTS_GROUP_LOGIN_LIMIT,
                ALK_MODE_PERMISSION_USERS,
                args[LOGIN_LIMIT_ARG_PERMISSION_USERS],
            )
        try:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "wb"))
        except Exception:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "w"))
        permission_users = args[LOGIN_LIMIT_ARG_PERMISSION_USERS].split(";")
        self.set_permission_user(permission_users)

        if not args[LOGIN_LIMIT_ARG_ENABLED]:
            return (True, "")
        for pwdent in pwd.getpwall():
            if (
                (not self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell))
                or BUILTIN_PERMISSION_USERS.__contains__(pwdent.pw_name)
                or permission_users.__contains__(pwdent.pw_name)
            ):
                continue
            if not self.is_nologin_shell(pwdent.pw_shell):
                br.utils.subprocess_not_output(
                    "usermod -s /sbin/nologin {0}".format(pwdent.pw_name)
                )

        return (True, "")

    def backup(self):
        retdata = dict()

        # 记录允许登录账号
        permission_value = self.conf.get(
            ACCOUNTS_GROUP_LOGIN_LIMIT, ALK_MODE_PERMISSION_USERS
        )
        if permission_value:
            retdata[LOGIN_LIMIT_ARG_PERMISSION_USERS] = permission_value

        # 记录不允许登录账号
        permission_users = permission_value.split(";")
        for pwdent in pwd.getpwall():
            if (
                (not self.is_human(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell))
                or BUILTIN_PERMISSION_USERS.__contains__(pwdent.pw_name)
                or permission_users.__contains__(pwdent.pw_name)
            ):
                continue
            # 其他账号允许登录情况
            accounts_allow = list()
            accounts_not_allow = list()
            if self.is_nologin_shell(pwdent.pw_shell):
                accounts_not_allow.append(pwdent.pw_name)
            else:
                accounts_allow.append(pwdent.pw_name)
            if accounts_allow:
                retdata[ACCOUNTS_ALLOW_LOGIN] = ";".join(accounts_allow)
            if accounts_not_allow:
                retdata[ACCOUNTS_NOT_ALLOW_LOGIN] = ";".join(accounts_not_allow)

        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        args = json.loads(args_json)
        # 配置文件恢复
        if args[LOGIN_LIMIT_ARG_PERMISSION_USERS]:
            self.conf.set(
                ACCOUNTS_GROUP_LOGIN_LIMIT,
                ALK_MODE_PERMISSION_USERS,
                args[LOGIN_LIMIT_ARG_PERMISSION_USERS],
            )
        try:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "wb"))
        except Exception:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "w"))

        # 允许登录账号恢复
        permission_users = args[LOGIN_LIMIT_ARG_PERMISSION_USERS].split(";")
        self.set_permission_user(permission_users)

        # 其他账号允许情况恢复
        if ACCOUNTS_ALLOW_LOGIN in args:
            accounts_allow = args[ACCOUNTS_ALLOW_LOGIN].split(";")
            self.set_permission_user(accounts_allow)
        if ACCOUNTS_NOT_ALLOW_LOGIN in args:
            accounts_not_allow = args[ACCOUNTS_NOT_ALLOW_LOGIN].split(";")
            for account in accounts_not_allow:
                if not self.check_user_exists(account):
                    continue
                br.utils.subprocess_not_output(
                    "usermod -s /sbin/nologin {0}".format(account)
                )

        return (True, "")


class NullPassword(Accounts):
    def is_ignore_account(self, uid, name, shell):
        if not self.is_null_pw_human(uid, name, shell):
            return True
        if THREE_RIGHTS_USERS.__contains__(name):
            return True
        return False

    def get(self):
        retdata = dict()
        retdata[NULL_ARG_ENABLED] = True

        for pwdent in pwd.getpwall():
            if self.is_ignore_account(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell):
                continue
            if self.is_null_password(pwdent.pw_name):
                retdata[NULL_ARG_ENABLED] = False
                break

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[NULL_ARG_ENABLED]:
            null_password_account = []
            for pwdent in pwd.getpwall():
                if self.is_ignore_account(
                    pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell
                ):
                    continue
                if self.is_null_password(pwdent.pw_name):
                    null_password_account.append(pwdent.pw_name)
            if len(null_password_account):
                # 没有设置密码，加固失败
                br.log.warning(
                    "Please set a password for empty password account:"
                    + ",".join(null_password_account)
                )
                return (False, "Please set a password for empty password account")
        return (True, "")

    def backup(self):
        retdata = dict()
        for pwdent in pwd.getpwall():
            if self.is_ignore_account(pwdent.pw_uid, pwdent.pw_name, pwdent.pw_shell):
                continue
            if self.is_null_password(pwdent.pw_name):
                if not retdata.__contains__(NULL_PW_ACCOUNTS):
                    retdata[NULL_PW_ACCOUNTS] = pwdent.pw_name
                else:
                    retdata[NULL_PW_ACCOUNTS] += ";" + pwdent.pw_name

        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        args = json.loads(args_json)
        if args[NULL_PW_ACCOUNTS]:
            for account in args[NULL_PW_ACCOUNTS].split(";"):
                if not self.check_user_exists(account):
                    continue
                br.utils.subprocess_not_output("passwd -d " + account)

        return (True, "")


class SurplusUser:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(ACCOUNTS_INI_FILEPATH)

    def get(self):
        retdata = dict()
        retdata[SURPLUS_DELETE_ENABLED] = True
        retdata[SURPLUS_DELETE_USERS] = self.conf.get(
            ACCOUNTS_GROUP_SURPLUS, ALK_MODE_DELETE_USERS
        )
        delete_users = retdata[SURPLUS_DELETE_USERS].split(";")

        for pwdent in pwd.getpwall():
            if DEAFULT_DELETE_USERS.__contains__(
                pwdent.pw_name
            ) or delete_users.__contains__(pwdent.pw_name):
                retdata[SURPLUS_DELETE_ENABLED] = False
                break
            else:
                continue

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.conf.set(
            ACCOUNTS_GROUP_SURPLUS, ALK_MODE_DELETE_USERS, args[SURPLUS_DELETE_USERS]
        )
        try:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "wb"))
        except Exception:
            self.conf.write(open(ACCOUNTS_INI_FILEPATH, "w"))
        delete_users = args[SURPLUS_DELETE_USERS].split(";")

        for pwdent in pwd.getpwall():

            delete_flag = False

            if args[SURPLUS_DELETE_ENABLED]:
                if DEAFULT_DELETE_USERS.__contains__(
                    pwdent.pw_name
                ):
                    delete_flag = True
                
            if delete_users.__contains__(pwdent.pw_name):
                delete_flag = True
            
            if delete_flag:
                br.log.debug(str(pwdent.pw_name))
                if pwdent.pw_uid != 0:
                    br.utils.subprocess_not_output(
                        "userdel {0} &> /dev/null ||: ".format(pwdent.pw_name)
                    )
                    br.utils.subprocess_not_output(
                        "groupdel {0} &> /dev/null ||: ".format(pwdent.pw_name)
                    )

        return (True, "")

    def backup(self):
        return (True, "")

    def rollback(self, args_json):
        return (True, "")
