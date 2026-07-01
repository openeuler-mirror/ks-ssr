# -*- coding: utf-8 -*-

import json
import br.configuration

SU_PAM_CONF_PATH = "/etc/pam.d/su"

# 禁止wheel组以外的用户su root
SU_WHEEL_ARG_ENABLED = "enabled"
# 如果没有匹配到对应的行，则使用该默认行进行设置
SU_WHEEL_FALLBACK_LINE = "auth           required        pam_wheel.so use_uid"

SU_WHEEL_NEXT_MATCH_LINE = "auth\\s+substack\\s+system-auth"

# 限制sudo命令使用权限

SUDO_LIMIT_PATH = "/etc/sudoers"

SUDO_LIMIT_FALLBACK_LINE = "%wheel  ALL=(ALL)       ALL"
SUDO_LIMIT_SYS_LINE = "sysadm  ALL=(ALL)       ROLE=sysadm_r   TYPE=sysadm_t    NOPASSWD: ALL"
SUDO_LIMIT_SEC_LINE = "secadm  ALL=(ALL)       ROLE=secadm_r   TYPE=secadm_t    NOPASSWD: ALL"
SUDO_LIMIT_AUD_LINE = "audadm  ALL=(ALL)       ROLE=auditadm_r TYPE=auditadm_t  NOPASSWD: ALL"


class SuWheel:
    def __init__(self):
        self.conf = br.configuration.PAM(
            SU_PAM_CONF_PATH, "auth\\s+required\\s+pam_wheel.so")

    def get(self):
        retdata = dict()
        retdata[SU_WHEEL_ARG_ENABLED] = (len(self.conf.get_line()) != 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if (args[SU_WHEEL_ARG_ENABLED]):
            self.conf.set_line(SU_WHEEL_FALLBACK_LINE,
                               SU_WHEEL_NEXT_MATCH_LINE)
        else:
            self.conf.del_line()
        return (True, '')


class SudoLimits:
    def __init__(self):
        self.conf = br.configuration.PAM(SUDO_LIMIT_PATH, "%wheel\\s+ALL")
        # 三权用户权限，之后若需要限制可将注释关闭
        # self.conf_sys = br.configuration.PAM(SUDO_LIMIT_PATH, "sysadm\\s+ALL")
        # self.conf_sec = br.configuration.PAM(SUDO_LIMIT_PATH, "secadm\\s+ALL")
        # self.conf_aud = br.configuration.PAM(SUDO_LIMIT_PATH, "audadm\\s+ALL")

    def get_selinux_status(self):
        output = br.utils.subprocess_has_output("getenforce")
        br.log.debug(output)
        if str(output) == "Enforcing":
            return True
        else:
            return False

    def get(self):
        retdata = dict()
        retdata["enabled"] = (len(self.conf.get_line()) == 0)
        if self.get_selinux_status():
            retdata["enabled"] = (len(self.conf.get_line()) != 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if (args["enabled"]):
            self.conf.del_line()
            if self.get_selinux_status():
                self.conf.set_line(SUDO_LIMIT_FALLBACK_LINE,
                                   "## Same thing without a password")
            # else:
            #     self.conf_sys.del_line()
            #     self.conf_sec.del_line()
            #     self.conf_aud.del_line()
        else:
            if self.get_selinux_status():
                # self.conf.set_line(SUDO_LIMIT_FALLBACK_LINE,"## Same thing without a password")
                return (False, "Please close SELinux and use it!")
            # else:
                # self.conf_aud.set_line(SUDO_LIMIT_AUD_LINE,"")
                # self.conf_sec.set_line(SUDO_LIMIT_SEC_LINE,"audadm\\s+ALL")
                # self.conf_sys.set_line(SUDO_LIMIT_SYS_LINE,"secadm\\s+ALL")
            self.conf.set_line(SUDO_LIMIT_FALLBACK_LINE,
                               "## Same thing without a password")
        return (True, '')
