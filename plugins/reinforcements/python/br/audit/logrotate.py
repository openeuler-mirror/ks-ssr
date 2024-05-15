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
        rotate_value = rotate_config.get_value('rotate')
        retdata[ROTATE_ARG_ROTATE] = "" if not rotate_value else int(rotate_config.get_value('rotate'))
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        rotate_config = br.configuration.KV(LOGROTATE_CONF_PATH)
        rotate_config.set_value('rotate', "" if not str(args[ROTATE_ARG_ROTATE]) else int(args[ROTATE_ARG_ROTATE]))
        return (True, '')
