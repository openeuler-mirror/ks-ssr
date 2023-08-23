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

PERMISSIONS_INI_FILEPATH = ssr.vars.SSR_PLUGIN_PYTHON_ROOT_DIR + "/ssr/config/permissions.ini"
GROUP_PERMISSIONS = "Permissions"

MODE_FILE_LIST = "ModeFileList"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH

PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"

UMASK_LIMIT_PROFILE_PATH = '/etc/profile'
UMASK_LIMIT_BASHRC_PATH  = '/etc/bashrc'

UMASK_LIMIT_CMD = 'grep -r umask'

class PermissionSetting:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(PERMISSIONS_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(LOGFILE_GROUP_PERMISSIONS, LPK_MODE_FILE_LIST).split(';')
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
                if not os.access(mode_file, os.F_OK):
                    continue
                mode = os.stat(mode_file).st_mode
                if mode != (mode & ~EXCLUDE_MODE):
                    os.chmod(mode_file, mode & ~EXCLUDE_MODE)

        return (True, '')

class UmaskLimit:
    def get(self):
        retdata = dict()

        command = '{0} {1}'.format(UMASK_LIMIT_CMD, UMASK_LIMIT_PROFILE_PATH)
        output = ssr.utils.subprocess_has_output(command)

        retdata['umask'] = output[-3:]
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        command = '{0} {1}'.format(UMASK_LIMIT_CMD, UMASK_LIMIT_PROFILE_PATH)
        output = ssr.utils.subprocess_has_output(command)

        profile_cmd = 'sed -i \'s/umask {0}/umask {1}/g\' {2}'.format(output[-3:], args['umask'], UMASK_LIMIT_PROFILE_PATH)
        profile_output = ssr.utils.subprocess_not_output(profile_cmd)

        bashrc_cmd = 'sed -i \'s/umask {0}/umask {1}/g\' {2}'.format(output[-3:], args['umask'], UMASK_LIMIT_BASHRC_PATH)
        bashrc_output = ssr.utils.subprocess_not_output(bashrc_cmd)

        return (True, '')