try:
    import configparser
except:
    import ConfigParser as configparser

import os
import stat
import ssr.utils
import json
import ssr.vars

LOGFILE_INI_FILEPATH = ssr.vars.SSR_PLUGIN_PYTHON_ROOT_DIR + "/ssr/audit/logfile.ini"
LOGFILE_GROUP_PERMISSIONS = "Permissions"
# LPK: Logfile Permissions Key
LPK_MODE_FILE_LIST = "ModeFileList"
LPK_APPEND_FILE_LIST = "AppendFileList"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH

PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"
PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT = "append-permissions-limit"


class Permissions:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(LOGFILE_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(LOGFILE_GROUP_PERMISSIONS, LPK_MODE_FILE_LIST).split(';')
            self.append_filelist = self.conf.get(LOGFILE_GROUP_PERMISSIONS, LPK_APPEND_FILE_LIST).split(';')
        except Exception as e:
            self.mode_filelist = list()
            self.append_filelist = list()
            ssr.log.debug(str(e))

    def get(self):
        retdata = dict()

        mode_permissions_limit = True
        append_permissions_limit = True

        for mode_file in self.mode_filelist:
            if not os.access(mode_file, os.F_OK):
                continue
            mode = os.stat(mode_file).st_mode
            if (mode & EXCLUDE_MODE) != 0:
                mode_permissions_limit = False
                break
        retdata[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT] = mode_permissions_limit

        ssr.log.debug(str(self.mode_filelist))
        ssr.log.debug(str(self.append_filelist))

        for append_file in self.append_filelist:
            if not os.access(append_file, os.F_OK):
                continue
            file_attrs = ssr.utils.subprocess_has_output('lsattr -l {0}'.format(append_file))
            ssr.log.debug(file_attrs)
            if file_attrs.find("Append_Only") == -1:
                append_permissions_limit = False
                break
        ssr.log.debug(append_permissions_limit)
        retdata[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT] = append_permissions_limit

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

        if args[PERMISSIONS_ARG_APPEND_PERMISSIONS_LIMIT]:
            for append_file in self.append_filelist:
                if not os.access(append_file, os.F_OK):
                    continue
                ssr.utils.subprocess_not_output('chattr +a {0}'.format(append_file))
        return (True, '')