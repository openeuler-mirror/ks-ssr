# -*- coding: utf-8 -*-

import json
import br.utils

NMCLI_CMD_PATH = '/usr/bin/nmcli'


class NMClient:
    def wifi_is_enabled(self):
        command = '{0} radio wifi'.format(NMCLI_CMD_PATH)
        output = br.utils.subprocess_has_output(command)
        return output == 'enabled'

    def wwan_is_enabled(self):
        command = '{0} radio wwan'.format(NMCLI_CMD_PATH)
        output = br.utils.subprocess_has_output(command)
        return output == 'enabled'

    def enable_radio(self):
        command = '{0} radio all on'.format(NMCLI_CMD_PATH)
        br.utils.subprocess_not_output(command)

    def disable_radio(self):
        command = '{0} radio all off'.format(NMCLI_CMD_PATH)
        br.utils.subprocess_not_output(command)


class Switch(NMClient):
    def get(self):
        retdata = dict()
        retdata['enabled'] = not (
            self.wifi_is_enabled() or self.wwan_is_enabled())
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args['enabled']:
            self.disable_radio()
        else:
            self.enable_radio()

        return (True, '')
