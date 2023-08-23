#--coding:utf8 --

import json
import ssr.utils

NO_USER_FILES_CMD_PATH = 'find / -nouser'

class NoUserFiles:
    def search_no_user_files(self):
        command = '{0}'.format(NO_USER_FILES_CMD_PATH)
        output = ssr.utils.subprocess_has_output(command)
        return output == NULL

    def delete_no_user_files(self):
        command = '{0} rm -rf \{\} +'.format(NO_USER_FILES_CMD_PATH)
        output = ssr.utils.subprocess_not_output(command)

class Switch(NoUserFiles):
    def get(self):
        retdata = dict()
        retdata['enabled'] = self.search_no_user_files()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args['enabled']:
            self.search_no_user_files()
        else:
            if self.search_no_user_files() != NULL:
                self.delete_no_user_files()

        return (True, '')
