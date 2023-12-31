#--coding:utf8 --

try:
    import configparser
except:
    import ConfigParser as configparser

import os
import stat
import ssr.utils
import json
import ssr.vars
import ssr.configuration

PERMISSIONS_INI_FILEPATH = ssr.vars.SSR_PLUGIN_PYTHON_ROOT_DIR + "/ssr/config/permissions.ini"
FILE_GROUP_PERMISSIONS = "Permissions"
# FPK: File Permissions Key
FPK_MODE_FILE_LIST = "ModeFileList"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH | stat.S_IXUSR

PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"

UMASK_LIMIT_PROFILE_PATH = '/etc/profile'
UMASK_LIMIT_BASHRC_PATH  = '/etc/bashrc'

UMASK_LIMIT_CONF_KEY_UMASK = 'umask'

class PermissionSetting:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(PERMISSIONS_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(FILE_GROUP_PERMISSIONS, FPK_MODE_FILE_LIST).split(';')
        except Exception as e:
            self.mode_filelist = list()
            ssr.log.debug(str(e))

    def get(self):
        retdata = dict()

        mode_permissions_limit = True

        for mode_file in self.mode_filelist:
            if not os.access(mode_file, os.F_OK):
                continue
            mode = os.stat(mode_file).st_mode
            if (mode & EXCLUDE_MODE) != 0:
                mode_permissions_limit = False
                break
        retdata[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT] = mode_permissions_limit

        ssr.log.debug(str(self.mode_filelist))

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT]:
            for mode_file in self.mode_filelist:
                ssr.log.debug(str(mode_file))
                if not os.access(mode_file, os.F_OK):
                    continue
                mode = os.stat(mode_file).st_mode
                if mode != (mode & ~EXCLUDE_MODE):
                    os.chmod(mode_file, mode & ~EXCLUDE_MODE)

        return (True, '')

class UmaskLimit:
    def __init__(self):
        self.conf_profile = ssr.configuration.KV(UMASK_LIMIT_PROFILE_PATH)
        self.conf_bashrc = ssr.configuration.KV(UMASK_LIMIT_BASHRC_PATH)

    def get(self):
        retdata = dict()

        profile_value = self.conf_profile.get_value(UMASK_LIMIT_CONF_KEY_UMASK)
        bashrc_vale = self.conf_bashrc.get_value(UMASK_LIMIT_CONF_KEY_UMASK)

        if profile_value == bashrc_vale and profile_value == '027':
            retdata['enabled'] = True
        else:
            retdata['enabled'] = False

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args['enabled']:
            self.conf_profile.set_all_value(UMASK_LIMIT_CONF_KEY_UMASK, '027')
            self.conf_bashrc.set_all_value(UMASK_LIMIT_CONF_KEY_UMASK, '027')
        else:
            self.conf_profile.set_all_value(UMASK_LIMIT_CONF_KEY_UMASK, '077')
            self.conf_bashrc.set_all_value(UMASK_LIMIT_CONF_KEY_UMASK, '077')

        cmd = "source" + " " + UMASK_LIMIT_BASHRC_PATH + " " + UMASK_LIMIT_PROFILE_PATH
        limit_open_command = '{0}'.format(cmd)
        open_output = ssr.utils.subprocess_not_output(limit_open_command)

        return (True, '')