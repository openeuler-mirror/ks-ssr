#--coding:utf8 --

import json
import ssr.utils

FILE_PERMISSIONS_KEY_PASSWD = "passwd"
FILE_PERMISSIONS_KEY_BASHRC = "bashrc"
FILE_PERMISSIONS_KEY_SHADOW = "shadow"
FILE_PERMISSIONS_KEY_GROUP  = "group"
FILE_PERMISSIONS_KEY_SERVICES = "services"
FILE_PERMISSIONS_KEY_XINETD = "xinetd"
FILE_PERMISSIONS_KEY_SECURITY = "security"

KEY_FILE_PASSWD = "/etc/passwd"
KEY_FILE_BASHRC  = "/etc/bashrc"
KEY_FILE_SHADOW  = "/etc/shadow"
KEY_FILE_GROUP  = "/etc/group"
KEY_FILE_SERVICES  = "/etc/services"
KEY_FILE_XINETD  = "/etc/xinetd.conf"
KEY_FILE_SECURITY  = "/etc/security"

FILE_PERMISSIONS_GET_CMD = "stat --format=\"%a\""
FILE_PERMISSIONS_SET_CMD = "chmod"

class PermissionSetting:
    def get(self):
        retdata = dict()

        passwd_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_PASSWD)
        bashrc_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_BASHRC)
        shadow_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_SHADOW)
        group_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_GROUP)
        services_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_SERVICES)
        xinetd_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_XINETD)
        security_cmd = '{0} {1}'.format(FILE_PERMISSIONS_GET_CMD, KEY_FILE_SECURITY)

        retdata[FILE_PERMISSIONS_KEY_PASSWD] = ssr.utils.subprocess_has_output(passwd_cmd)
        retdata[FILE_PERMISSIONS_KEY_BASHRC] = ssr.utils.subprocess_has_output(bashrc_cmd)
        retdata[FILE_PERMISSIONS_KEY_SHADOW] = ssr.utils.subprocess_has_output(shadow_cmd)
        retdata[FILE_PERMISSIONS_KEY_GROUP] = ssr.utils.subprocess_has_output(group_cmd)
        retdata[FILE_PERMISSIONS_KEY_SERVICES] = ssr.utils.subprocess_has_output(services_cmd)
        retdata[FILE_PERMISSIONS_KEY_XINETD] = ssr.utils.subprocess_has_output(xinetd_cmd)
        retdata[FILE_PERMISSIONS_KEY_SECURITY] = ssr.utils.subprocess_has_output(security_cmd)

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        passwd_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_PASSWD], KEY_FILE_PASSWD)
        bashrc_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_BASHRC], KEY_FILE_PASSWD)
        shadow_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_SHADOW], KEY_FILE_PASSWD)
        group_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_GROUP], KEY_FILE_PASSWD)
        services_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_SERVICES], KEY_FILE_PASSWD)
        xinetd_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_XINETD], KEY_FILE_PASSWD)
        security_cmd = '{0} {1} {2}'.format(FILE_PERMISSIONS_SET_CMD, args[FILE_PERMISSIONS_KEY_SECURITY], KEY_FILE_PASSWD)

        ssr.utils.subprocess_not_output(passwd_cmd)
        ssr.utils.subprocess_not_output(bashrc_cmd)
        ssr.utils.subprocess_not_output(shadow_cmd)
        ssr.utils.subprocess_not_output(group_cmd)
        ssr.utils.subprocess_not_output(services_cmd)
        ssr.utils.subprocess_not_output(xinetd_cmd)
        ssr.utils.subprocess_not_output(security_cmd)

        return (True, '')
