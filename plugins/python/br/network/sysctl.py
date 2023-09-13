#--coding:utf8 --

import json
import br.configuration
import br.log

SYSCTL_PATH = '/usr/sbin/sysctl'

#SYSCTL_CONFI_FILE = "/etc/sysctl.d/90-br-network.conf"
SYSCTL_CONFI_FILE = "/etc/sysctl.conf"
SYSCTL_CONFIG_FIELD_PARTTERN = "\\s*=\\s*"
SYSCTL_ACCEPT_REDIRECTS_PATTERN = "net.ipv4.conf.*.accept_redirects"
SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN = "net.ipv4.conf.*.accept_source_route"

class Sysctl:
    def get_items_by_pattern(self, partten):
        output = br.utils.subprocess_has_output('{0} -a -r {1}'.format(SYSCTL_PATH, partten))
        lines = output.split('\n')
        retval = list()
        for line in lines:
            fields = line.split('=')
            if len(fields) != 2:
                continue
            retval.append((fields[0].strip(), fields[1].strip()))
        return retval

    def load_from_system(self):
        br.utils.subprocess_not_output('{0} --system'.format(SYSCTL_PATH))


# ICMP重定向
class IcmpRedirect(Sysctl):
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
        sysctl_config = br.configuration.KV(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN, ' = ')
        enabled = args['enabled']

        for redirect_item in redirect_items:
            sysctl_config.set_all_value(redirect_item[0], "1" if enabled else "0")

        # 从文件中刷新
        self.load_from_system()
        return (True, '')


# IP源路由
class SourceRoute(Sysctl):
    def get(self):
        retdata = dict()
        source_route_items = self.get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)

        enabled = False
        for source_route_item in source_route_items:
            if source_route_item[1] == "1":
                enabled = True
                break

        retdata['enabled'] = enabled
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        redirect_items = self.get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)
        sysctl_config = br.configuration.KV(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN, ' = ')
        enabled = args['enabled']

        for redirect_item in redirect_items:
            sysctl_config.set_all_value(redirect_item[0], "1" if enabled else "0")

        # 从文件中刷新
        self.load_from_system()
        return (True, '')


# Syn flood攻击
class SynFlood(Sysctl):
    def get(self):
        retdata = dict()
        retdata['enabled'] = (br.utils.subprocess_has_output('sysctl -a -r net.ipv4.tcp_syncookies -n') == '1')
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        sysctl_config = br.configuration.KV(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN, ' = ')
        enabled = args['enabled']
        sysctl_config.set_all_value("net.ipv4.tcp_syncookies", "1" if enabled else "0")

        # 从文件中刷新
        self.load_from_system()
        return (True, '')