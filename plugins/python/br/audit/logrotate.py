# -*- coding: utf-8 -*-

import json
import br.configuration

LOGROTATE_CONF_PATH = "/etc/logrotate.conf"
ROTATE_ARG_ROTATE = "rotate"


# 日志文件保存周期
class Rotate:
    def get(self):
        retdata = dict()
        rotate_config = br.configuration.KV(LOGROTATE_CONF_PATH)
        retdata[ROTATE_ARG_ROTATE] = int(rotate_config.get_value('rotate'))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        rotate_config = br.configuration.KV(LOGROTATE_CONF_PATH)
        rotate_config.set_value('rotate', int(args[ROTATE_ARG_ROTATE]))
        return (True, '')
