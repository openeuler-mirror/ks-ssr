# -*- coding: utf-8 -*-

import json
import br.configuration
import br.systemd
import br.log

SSHD_CONF_PATH = "/etc/ssh/sshd_config"
SELINUX_MODULES_PORT_PATH = "/usr/share/ks-ssr/br-sshd-port.pp"
SELINUX_MODULES_SFTP_PATH = "/usr/share/ks-ssr/br-sshd-sftp.pp"

# 允许root进行ssh远程登陆
ROOT_LOGIN_ARG_ENABLED = "enabled"
PUBKEY_AUTH_ARG_ENABLED = "enabled"
WEAK_ENCRYPT_ARG_ENABLED = "enabled"
BANNER_INFO_ARG_ENABLED = "enabled"

BANNER_INFO_KEY = "Banner"

# 会话登录后超过指定时间未操作则自动退出
PROFILE_CLIENT_TMOUT = "ClientAliveInterval"
PROFILE_CLIENT_COUNT = "ClientAliveCountMax"

# 用于匹配TMOUT进行修改
PROFILE_TMOUT = "TMOUT"
PROFILE_TMOUT_RXPORT = "export TMOUT"

DEFAULT_CIPHERS = ("aes128-ctr", "aes192-ctr",
                   "aes256-ctr", "aes128-cbc", "3des-cbc")
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

SET_SFTP_USER_CMD = "useradd  -d /home/sftpuser -s /sbin/nologin sftpuser;chown root:sftpuser /home/sftpuser;chmod 755 /home/sftpuser;mkdir /home/sftpuser/test;chown sftpuser:sftpuser /home/sftpuser/test"
SET_SFTP_USER_CONFIG = "Match User sftpuser\n\tChrootDirectory /home/sftpuser\n\tX11Forwarding no\n\tAllowTcpForwarding no\n\tForceCommand internal-sftp"

ERROR_NOTIFY = "sshd.services is not running!"

class SSHD:
    def __init__(self):
        self.conf = br.configuration.KV(SSHD_CONF_PATH, join_string=" ")
        self.conf_profile = br.configuration.KV("/etc/profile", "=", "=")
        self.conf_bashrc = br.configuration.KV("/etc/bashrc", "=", "=")
        self.conf_ciphers = br.configuration.PAM(
            SSHD_CONF_PATH, "Ciphers\\s+")
        self.conf_protocol = br.configuration.PAM(
            SSHD_CONF_PATH, "Protocol\\s+2")
        self.service = br.systemd.Proxy("sshd")

    def get_selinux_status(self):
        output = br.utils.subprocess_has_output("getenforce")
        br.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False


class RootLogin(SSHD):
    def get(self):
        retdata = dict()
        retdata[ROOT_LOGIN_ARG_ENABLED] = (
            self.conf.get_value("PermitRootLogin") == "no")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)
        args = json.loads(args_json)
        self.conf.set_all_value(
            "PermitRootLogin", "no" if args[ROOT_LOGIN_ARG_ENABLED] else "yes")
        # 重启服务生效
        self.service.reload()
        return (True, '')


class PubkeyAuth(SSHD):
    def get(self):
        retdata = dict()
        retdata[PUBKEY_AUTH_ARG_ENABLED] = (
            not (self.conf.get_value("PubkeyAuthentication") == "no"))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)
        args = json.loads(args_json)
        self.conf.set_all_value(
            "PubkeyAuthentication", "yes" if args[PUBKEY_AUTH_ARG_ENABLED] else "no")
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
        args = json.loads(args_json)
        ciphers = self.conf.get_value("Ciphers").split(",")
        # 过滤空元素
        if all(ciphers) and len(ciphers) > 1:
            ciphers = filter(lambda x: x, ciphers)

        # 这里只处理不允许弱加密算法的情况。如果是允许弱加密算法，这里也不会把弱加密算法添加进去，因为这会导致新版本sshd无法启动
        if not args[WEAK_ENCRYPT_ARG_ENABLED]:
            # 如果未空说明是默认配置，新版本中默认配置是不支持若加密算法，因此只考虑非默认配置的情况
            if (len(ciphers) > 0):
                ciphers = [
                    cipher for cipher in ciphers if cipher.find("arcfour") == -1]
            # 如果列表为空，则改为默认值
            if len(ciphers) == 0:
                self.conf.del_record("Ciphers")
            else:
                self.conf.set_all_value("Ciphers", ','.join(ciphers))
                br.log.debug("Ciphers " + ",".join(ciphers))
            # 重启服务生效
            self.service.reload()
        return (True, '')


class BannerInfo(SSHD):
    def get(self):
        retdata = dict()
        banner = self.conf.get_value("Banner")
        retdata[BANNER_INFO_KEY] = "" if not banner else str(banner)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)
        args = json.loads(args_json)
        self.conf.set_all_value("Banner", args[BANNER_INFO_KEY])
        # 重启服务生效
        self.service.reload()
        return (True, '')


class SessionTimeout(SSHD):
    def set_conf_value(self, arg):
        if (arg <= 0):
            self.conf.del_record(PROFILE_CLIENT_TMOUT)
            self.conf.del_record(PROFILE_CLIENT_COUNT)
        else:
            if self.conf.get_value(PROFILE_CLIENT_TMOUT) != str(arg):
                self.conf.set_all_value(PROFILE_CLIENT_TMOUT, arg)
            if self.conf.get_value(PROFILE_CLIENT_COUNT) != 0:
                self.conf.set_all_value(PROFILE_CLIENT_COUNT, 0)
    # 如果/etc/profile /etc/bashrc 中有TMOUT的值，则进行修改
    def set_conf_profile_value(self, arg):
        if (arg <= 0):
            self.conf_profile.set_all_value(PROFILE_TMOUT, "")
            self.conf_profile.set_all_value(PROFILE_TMOUT_RXPORT, "")
        else:
            if self.conf_profile.get_value(PROFILE_TMOUT) or len(br.utils.subprocess_has_output("cat /etc/profile |grep '#{0}'".format(PROFILE_TMOUT))):
                self.conf_profile.set_all_value(PROFILE_TMOUT, arg)
            if self.conf_profile.get_value(PROFILE_TMOUT_RXPORT) or len(br.utils.subprocess_has_output("cat /etc/profile |grep '#{0}'".format(PROFILE_TMOUT_RXPORT))):
                self.conf_profile.set_all_value(PROFILE_TMOUT_RXPORT, arg)
    def set_conf_bashrc_value(self, arg):
        if (arg <= 0):
            self.conf_bashrc.set_all_value(PROFILE_TMOUT, "")
            self.conf_bashrc.set_all_value(PROFILE_TMOUT_RXPORT, "")
        else:
            if self.conf_bashrc.get_value(PROFILE_TMOUT) or len(br.utils.subprocess_has_output("cat /etc/bashrc |grep '#{0}'".format(PROFILE_TMOUT))):
                self.conf_bashrc.set_all_value(PROFILE_TMOUT, arg)
            if self.conf_bashrc.get_value(PROFILE_TMOUT_RXPORT) or len(br.utils.subprocess_has_output("cat /etc/bashrc |grep '#{0}'".format(PROFILE_TMOUT_RXPORT))):
                self.conf_bashrc.set_all_value(PROFILE_TMOUT_RXPORT, arg)

    def get(self):
        retdata = dict()
        timeout = self.conf.get_value(PROFILE_CLIENT_TMOUT)
        if not timeout:
            timeout='0'
        retdata[PROFILE_CLIENT_TMOUT] = int(timeout)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)
        args = json.loads(args_json)

        self.set_conf_value(args[PROFILE_CLIENT_TMOUT])
        self.set_conf_profile_value(args[PROFILE_CLIENT_TMOUT])
        self.set_conf_bashrc_value(args[PROFILE_CLIENT_TMOUT])

        # 重启服务生效
        self.service.reload()
        return (True, '')


class SshdService(SSHD):
    def clear_port(self):
        output = br.utils.subprocess_has_output("semanage port -l |grep ssh")
        port_list = output.replace(' ', '').split("tcp")[1].split(",")
        for port in port_list:
            if port == "22" or port == "":
                continue
            br.utils.subprocess_has_output_ignore_error_handling(
                "semanage port -d -t ssh_port_t -p tcp {0}".format(port))

    def get(self):
        retdata = dict()
        retdata[SSHD_CONF_PROTOCOL_KEY] = (
            self.conf.get_value(SSHD_CONF_PROTOCOL) == "2")
        retdata[SSHD_CONF_PERMIT_EMPTY_KEY] = self.conf.get_value(
            SSHD_CONF_PERMIT_EMPTY) == "no"
        retdata[SSHD_CONF_PAM_KEY] = (
            not (self.conf.get_value(SSHD_CONF_PAM) == "no"))
        port = self.conf.get_value(SSHD_CONF_PORT)
        retdata[SSHD_CONF_PORT_KEY] = "" if not port else port
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)

        if args[SSHD_CONF_PAM_KEY]:
            self.conf.set_all_value(SSHD_CONF_PAM, "yes")
        else:
            return (False, "UsePAM is not recommended to be closed,\nwhich will cause many problems!")

        if args[SSHD_CONF_PROTOCOL_KEY]:
            self.conf_protocol.set_line("{0} 2".format(
                SSHD_CONF_PROTOCOL), "#\\s+no\\s+default\\s+banner")
        else:
            self.conf_protocol.del_line()

        self.conf.set_all_value(
            SSHD_CONF_PERMIT_EMPTY, "no" if args[SSHD_CONF_PERMIT_EMPTY_KEY] else "yes")

        self.clear_port()
        if args[SSHD_CONF_PORT_KEY] != "" and args[SSHD_CONF_PORT_KEY] != "\"\"":
            self.conf.set_all_value(SSHD_CONF_PORT, args[SSHD_CONF_PORT_KEY])
            if self.get_selinux_status():
                br.utils.subprocess_not_output(
                    "semodule -i {0}".format(SELINUX_MODULES_PORT_PATH))
                br.utils.subprocess_not_output(
                    "semanage port -a -t ssh_port_t -p tcp {0}".format(args[SSHD_CONF_PORT_KEY]))
        else:
            self.conf.set_all_value(SSHD_CONF_PORT, "")
            if self.get_selinux_status():
                br.utils.subprocess_not_output(
                    "semodule -r br-sshd-port &> /dev/null")

        # 重启服务生效
        self.service.reload()
        return (True, '')


class SftpUser(SSHD):
    def user_exist(self, username):
        cmd = 'if id -u {0} >/dev/null 2>&1 ; then echo exist; fi'.format(
            username)
        if len(br.utils.subprocess_has_output(cmd)) == 0:
            return False
        else:
            return True

    def get(self):
        retdata = dict()
        retdata["enabled"] = self.user_exist("sftpuser")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        if not self.service.is_active():
            return (False, ERROR_NOTIFY)
        self.conf_sub = br.configuration.PAM(
            SSHD_CONF_PATH, "Subsystem\\s+sftp\\s+/usr/libexec/openssh/sftp-server")
        self.conf_sub_tinternal = br.configuration.PAM(
            SSHD_CONF_PATH, "Subsystem\\s+sftp\\s+internal-sftp")
        self.conf_match = br.configuration.PAM(
            SSHD_CONF_PATH, "Match\\s+User\\s+sftpuser")
        self.conf_chroot = br.configuration.PAM(
            SSHD_CONF_PATH, "\\s+ChrootDirectory\\s+/home")
        self.conf_x11 = br.configuration.PAM(
            SSHD_CONF_PATH, "\\s+X11Forwarding\\s+no\\s+#")
        self.conf_allowtcp = br.configuration.PAM(
            SSHD_CONF_PATH, "\\s+AllowTcpForwarding\\s+no\\s+#")
        self.conf_force = br.configuration.PAM(
            SSHD_CONF_PATH, "\\s+ForceCommand\\s+internal-sftp")
        args = json.loads(args_json)

        if args["enabled"]:
            if not self.user_exist("sftpuser"):
                br.utils.subprocess_not_output(SET_SFTP_USER_CMD)

            self.conf_sub_tinternal.set_line(
                "Subsystem\tsftp\tinternal-sftp", "Subsystem\\s+sftp\\s+/usr/libexec/openssh/sftp-server")
            self.conf_sub.del_line()
            # self.conf_force.set_line("\tForceCommand internal-sftp","#\\s+PermitTTY\\s+no")
            if len(br.utils.subprocess_has_output("cat {0} |grep 'ForceCommand internal-sftp' ".format(SSHD_CONF_PATH))) != 0:
                self.conf_force.set_line("\tForceCommand internal-sftp", "")
            else:
                self.conf.set_value("\tForceCommand", "internal-sftp")
            self.conf_allowtcp.set_line(
                "\tAllowTcpForwarding no #SSR configuration, TCP forwarding forbidden", "\tForceCommand internal-sftp")
            self.conf_x11.set_line(
                "\tX11Forwarding no #SSR configuration, X11 forwarding forbidden", "\tAllowTcpForwarding\\s+no\\s+#")
            self.conf_chroot.set_line(
                "\tChrootDirectory /home/sftpuser", "\tX11Forwarding\\s+no\\s+#")
            self.conf_match.set_line(
                "Match User sftpuser", "\tChrootDirectory\\s+/home/sftpuser")

            if self.get_selinux_status():
                br.utils.subprocess_not_output(
                    "semodule -i {0}".format(SELINUX_MODULES_SFTP_PATH))

        else:
            if self.user_exist("sftpuser"):
                rm_cmd = "userdel -r sftpuser &> /dev/null;rm -rf /home/sftpuser"
                br.utils.subprocess_not_output(rm_cmd)

            self.conf_sub.set_line(
                "Subsystem\tsftp\t/usr/libexec/openssh/sftp-server", "Subsystem\\s+sftp\\s+internal-sftp")
            self.conf_sub_tinternal.del_line()
            self.conf_match.del_line()
            self.conf_chroot.del_line()
            self.conf_x11.del_line()
            self.conf_allowtcp.del_line()
            self.conf_force.del_line()

            if self.get_selinux_status():
                br.utils.subprocess_not_output(
                    "semodule -r br-sshd-sftp &> /dev/null")
        # 重启服务生效
        self.service.reload()
        return (True, '')
