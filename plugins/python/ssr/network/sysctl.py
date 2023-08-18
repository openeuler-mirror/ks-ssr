#--coding:utf8 --

import json
import ssr.configuration

SYSCTL_PATH = '/usr/sbin/sysctl'

SYSCTL_CONFI_FILE = "/etc/sysctl.d/10-sysctl-ssr.conf"
SYSCTL_CONFIG_FIELD_PARTTERN = "\\s*=\\s*"
SYSCTL_ACCEPT_REDIRECTS_PATTERN = "net.ipv4.conf.*.accept_redirects"
SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN = "net.ipv4.conf.*.accept_source_route"


class Sysctl:
    def get_items_by_pattern(self, partten):
        output = ssr.utils.subprocess_has_output('{0} -a -r {1}'.format(SYSCTL_PATH, partten))
        lines = output.split('\n')
        retval = list()
        for line in lines:
            fields = line.split()
            if len(fields) != 2:
                continue
            retval.append((fields[0], fields[1]))
        return retval


class Redirect(Sysctl):
    def get(self):
        retdata = dict()
        redirect_items = self.get_items_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN)

        enabled = False
        for redirect_item in redirect_items:
            if redirect_item[1] == "1":
                enabled = True
                break

        retdata['enabled'] = enabled
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        redirect_items = self.get_items_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN)
        sysctl_config = ssr.configuration.Plain(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN)
        enabled = args['enabled']

        for redirect_item in redirect_items:
            sysctl_config.set_value(redirect_item, "1" if enabled else "0")

        # 从文件中刷新
        ssr.utils.subprocess_not_output('{0} --load'.format(SYSCTL_PATH))
        return (True, '')


class SourceRoute(Sysctl):
    def get(self):
        retdata = dict()
        redirect_items = self.get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)

        enabled = False
        for redirect_item in redirect_items:
            if redirect_item[1] == "1":
                enabled = True
                break

        retdata['enabled'] = enabled
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        redirect_items = self.get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)
        sysctl_config = ssr.configuration.Plain(SYSCTL_CONFI_FILE)
        enabled = args['enabled']

        for redirect_item in redirect_items:
            sysctl_config.set_value(redirect_item, "1" if enabled else "0")

        # 从文件中刷新
        ssr.utils.subprocess_not_output('{0} --load'.format(SYSCTL_PATH))
        return (True, '')