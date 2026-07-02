#--coding:utf8 --

import json
import ssr.utils

UMASK_LIMIT_PROFILE_PATH = '/etc/profile'
UMASK_LIMIT_BASHRC_PATH  = '/etc/bashrc'

UMASK_LIMIT_CMD = 'grep -r umask'

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
