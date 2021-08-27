#--coding:utf8 --

import json
import ssr.config

LOGROTATE_CONF_PATH = "/etc/logrotate.conf"


class Rotate:
    def get(self):
        retdata = dict()
        rotate_config = ssr.config.Plain(LOGROTATE_CONF_PATH)
        retdata['rotate'] = int(rotate_config.get_value('rotate'))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        rotate_config = ssr.config.Plain(LOGROTATE_CONF_PATH)
        rotate_config.set_value('rotate', int(args['rotate']))
        return (True, '')