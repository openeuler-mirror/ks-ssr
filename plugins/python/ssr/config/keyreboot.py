#--coding:utf8 --

import json
import ssr.utils

COMPOSITE_KEY_REBOOT_CONF_PATH = "/usr/lib/systemd/system/reboot.target"

COMPOSITE_KEY_REBOOT_CONF ='''
#  SPDX-License-Identifier: LGPL-2.1+
#
#  This file is part of systemd.
#
#  systemd is free software; you can redistribute it and/or modify it
#  under the terms of the GNU Lesser General Public License as published by
#  the Free Software Foundation; either version 2.1 of the License, or
#  (at your option) any later version.

[Unit]
Description=Reboot
Documentation=man:systemd.special(7)
DefaultDependencies=no
Requires=systemd-reboot.service
After=systemd-reboot.service
AllowIsolate=yes
JobTimeoutSec=30min
JobTimeoutAction=reboot-force

[Install]
Alias=ctrl-alt-del.target
'''

class CompositeKeyReboot:
    def search(self):
        command = 'find /usr/lib/systemd/system/ -name  {0}'.format("reboot.target")
        output = ssr.utils.subprocess_has_output(command)
        return len(output) == 0

    def delete(self):
        command = 'rm -rf {0}'.format(COMPOSITE_KEY_REBOOT_CONF_PATH)
        output = ssr.utils.subprocess_not_output(command)

    def create(self):
        command = 'echo \"{0}\" > {1}'.format(COMPOSITE_KEY_REBOOT_CONF, COMPOSITE_KEY_REBOOT_CONF_PATH)
        output = ssr.utils.subprocess_has_output(command)

class KeyRebootSwitch(CompositeKeyReboot):
    def get(self):
        retdata = dict()
        retdata['enabled'] = self.search()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if self.search():
            if args['enabled']:
                self.create()
        else:
            if not args['enabled']:
                self.delete()

        return (True, '')
