# -*- coding: utf-8 -*-

import json
import br.configuration
import br.utils
import br.log

# 扫描的路径
SCAN_FILES_PATH = ("/bin", "/usr/bin", "/sbin", "/usr/sbin")

SCAN_FILES_CMD = "find"

class NouserFiles:
    def __init__(self):
        self.is_scan = False

    def get(self):
        retdata = dict()
        retdata["enabled"] = False
        scan_output = ""
        if self.is_scan:
            for path in SCAN_FILES_PATH:
                scan_cmd = '{0} {1} -nouser'.format(SCAN_FILES_CMD, path)
                tmp_output = br.utils.subprocess_has_output(scan_cmd)
                if (len(tmp_output)):
                    tmp_output += '\n'
                scan_output += tmp_output
                # br.log.debug("scan_onuser_output = ")
                # br.log.debug(scan_output)
        if scan_output == "":
            retdata["nouser_files"] = "\n"
        else:
            retdata["nouser_files"] = scan_output

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args["enabled"]:
            self.is_scan = True
        else:
            self.is_scan = False

        return (True, '')


class AuthorityFiles:
    def __init__(self):
        self.is_scan = False

    def get(self):
        retdata = dict()
        retdata["enabled"] = False
        scan_output = ""
        if self.is_scan:
            for path in SCAN_FILES_PATH:
                scan_cmd = '{0} {1} -perm 777'.format(SCAN_FILES_CMD, path)
                tmp_output = br.utils.subprocess_has_output(scan_cmd)
                if (len(tmp_output)):
                    tmp_output += '\n'
                scan_output += tmp_output
                # br.log.debug("scan_777_files_output = ")
                # br.log.debug(scan_output)

        if scan_output == "":
            retdata["authority_files"] = "\n"
        else:
            retdata["authority_files"] = scan_output

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args["enabled"]:
            self.is_scan = True
        else:
            self.is_scan = False

        return (True, '')


class SuidSgidFiles:
    def __init__(self):
        self.is_scan = False

    def get(self):
        retdata = dict()
        retdata["enabled"] = False
        scan_suid_output = ""
        scan_guid_output = ""
        if self.is_scan:
            for path in SCAN_FILES_PATH:
                # /4000表示SUID权限 ，/2000表示GUID权限
                scan_suid_cmd = '{0} {1} -perm /4000'.format(
                    SCAN_FILES_CMD, path)
                scan_guid_cmd = '{0} {1} -perm /2000'.format(
                    SCAN_FILES_CMD, path)
                tmp_suid_output = br.utils.subprocess_has_output(
                    scan_suid_cmd)
                tmp_guid_output = br.utils.subprocess_has_output(
                    scan_guid_cmd)
                if (len(tmp_suid_output)):
                    tmp_suid_output += '\n'
                if (len(tmp_guid_output)):
                    tmp_guid_output += '\n'

                scan_suid_output += tmp_suid_output
                scan_guid_output += tmp_guid_output

            retdata["suid_sgid_files"] = "[  SUID ]\n" + \
                scan_suid_output + "[  GUID ]\n" + scan_guid_output
        else:
            retdata["suid_sgid_files"] = "\n"
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args["enabled"]:
            self.is_scan = True
        else:
            self.is_scan = False

        return (True, '')