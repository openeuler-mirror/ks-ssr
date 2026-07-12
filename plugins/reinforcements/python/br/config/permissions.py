# -*- coding: utf-8 -*-

try:
    import configparser
except Exception:
    import ConfigParser as configparser

import os
import stat
import br.utils
import json
import br.vars
import br.configuration

PERMISSIONS_INI_FILEPATH = (
    br.vars.SSR_BR_PLUGIN_PYTHON_ROOT_DIR + "/br/config/permissions.ini"
)
FILE_GROUP_PERMISSIONS = "Permissions"
# FPK: File Permissions Key
FPK_MODE_FILE_LIST = "ModeFileList"
# FPK: Directory Permissions Key
FPK_MODE_DIRECTORY_LIST = "ModeDirectoryList"

EXCLUDE_MODE = stat.S_IWGRP | stat.S_IXGRP | stat.S_IWOTH | stat.S_IXOTH | stat.S_IXUSR

EXCLUDE_DIRECTORY_MODE = (
    stat.S_IRWXU | stat.S_IXGRP | stat.S_IRGRP | stat.S_IROTH | stat.S_IXOTH
)

PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT = "mode-permissions-limit"
PERMISSIONS_ARG_MODE_DIRECTORY_PERMISSIONS_LIMIT = "directory-permissions-limit"
ST_MODE = "st-mode"

UMASK_PROFILE_SH_PATH = "/etc/profile.d/br-config-umask.sh"
UMASK_PROFILE_CSH_PATH = "/etc/profile.d/br-config-umask.csh"

def get_mode(mode_files):
    st_mode = ""
    for mode_file in mode_files:
        if not os.access(mode_file, os.F_OK):
            continue
        mode = os.stat(mode_file).st_mode
        if st_mode:
            st_mode += ";"
        st_mode += mode_file + ";" + str(mode)
    return st_mode


def set_mode(mode_data):
    mode_info = mode_data.split(";")
    for i in range(0, len(mode_info), 2):
        if i + 1 < len(mode_info):
            if not os.access(mode_info[i], os.F_OK):
                continue
            os.chmod(mode_info[i], int(mode_info[i+1]))


class PermissionSetting:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(PERMISSIONS_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(
                FILE_GROUP_PERMISSIONS, FPK_MODE_FILE_LIST
            ).split(";")
        except Exception as e:
            self.mode_filelist = list()
            br.log.debug(str(e))

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

        br.log.debug(str(self.mode_filelist))

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[PERMISSIONS_ARG_MODE_PERMISSIONS_LIMIT]:
            for mode_file in self.mode_filelist:
                br.log.debug(str(mode_file))
                if not os.access(mode_file, os.F_OK):
                    continue
                mode = os.stat(mode_file).st_mode
                if mode != (mode & ~EXCLUDE_MODE):
                    os.chmod(mode_file, mode & ~EXCLUDE_MODE)

        return (True, "")

    def backup(self):
        return self.get()

    def rollback(self, args_json):
        return self.set(args_json)


class DirectoryPermissionSetting:
    def __init__(self):
        self.conf = configparser.ConfigParser()
        self.conf.read(PERMISSIONS_INI_FILEPATH)
        try:
            self.mode_filelist = self.conf.get(
                FILE_GROUP_PERMISSIONS, FPK_MODE_DIRECTORY_LIST
            ).split(";")
        except Exception as e:
            self.mode_filelist = list()
            br.log.debug(str(e))

    def get(self):
        retdata = dict()

        mode_permissions_limit = True

        for mode_file in self.mode_filelist:
            if not os.access(mode_file, os.F_OK):
                continue
            mode = os.stat(mode_file).st_mode
            if (mode & EXCLUDE_DIRECTORY_MODE) != EXCLUDE_DIRECTORY_MODE:
                mode_permissions_limit = False
                break
        retdata[PERMISSIONS_ARG_MODE_DIRECTORY_PERMISSIONS_LIMIT] = (
            mode_permissions_limit
        )

        br.log.debug(str(self.mode_filelist))

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[PERMISSIONS_ARG_MODE_DIRECTORY_PERMISSIONS_LIMIT]:
            for mode_file in self.mode_filelist:
                br.log.debug(str(mode_file))
                if not os.access(mode_file, os.F_OK):
                    continue
                mode = os.stat(mode_file).st_mode
                if mode != (mode & EXCLUDE_DIRECTORY_MODE):
                    os.chmod(mode_file, EXCLUDE_DIRECTORY_MODE)

        return (True, "")

    def backup(self):
        return self.get()

    def rollback(self, args_json):
        return self.set(args_json)


class UmaskLimit:
    def __init__(self):
        pass

    # 判断文件是否已经存在umask配置
    def exist_umask(self, file):
        ret = br.utils.subprocess_has_output("grep \"^umask \" {0}".format(file))
        return len(ret) > 0


    def get(self):
        retdata = dict()
        retdata["umask"] = br.utils.subprocess_has_output("bash -l umask")
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if self.exist_umask("/etc/profile"):
            return (False, "umask is already defined in /etc/profile, please delete it first.")

        if self.exist_umask("/etc/bashrc"):
            return (False, "umask is already defined in /etc/bashrc, please delete it first.")

        if os.path.exists(UMASK_PROFILE_SH_PATH):
            os.remove(UMASK_PROFILE_SH_PATH)
        if os.path.exists(UMASK_PROFILE_CSH_PATH):
            os.remove(UMASK_PROFILE_CSH_PATH)

        if args["umask"] != None and len(args["umask"]) == 4:
            br.utils.subprocess_not_output(
                'echo "umask {0}" >> {1}'.format(args["umask"], UMASK_PROFILE_SH_PATH)
            )
            br.utils.subprocess_not_output(
                'echo "umask {0}" >> {1}'.format(args["umask"], UMASK_PROFILE_CSH_PATH)
            )
        return (True, "")

    def backup(self):
        retdata = dict()
        with open(UMASK_PROFILE_SH_PATH, "r") as file:
            lines = file.readlines()
            for line in lines:
                if line.startswith("umask"):
                    umask_value = line.split(" ")[1].strip()
                    retdata["umask"] = int(umask_value)
                    break
        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        return self.set(args_json)
