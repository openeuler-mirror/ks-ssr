#--coding:utf8 --

import json
import ssr.systemd
import ssr.log

FIREWALL_CMD_PATH = '/usr/bin/firewall-cmd'

# 禁止Traceroute探测
DISABLE_TRACEROUTE_CMD = "iptables -A INPUT -p ICMP --icmp-type time-exceeded -j DROP;  iptables -A OUTPUT -p ICMP --icmp-type time-exceeded -j DROP"
OPEN_TRACEROUTE_CMD = "iptables -D INPUT -p ICMP --icmp-type time-exceeded -j DROP;  iptables -D OUTPUT -p ICMP --icmp-type time-exceeded -j DROP"
STATUS_TRACEROUTE_CMD = "iptables -L -n |grep DROP |grep 11"

class Firewalld:
    def __init__(self):
        self.firewalld_systemd = ssr.systemd.Proxy('firewalld')

    def __has_port(self, port):
        command = '{0} --query-port={1} --permanent'.format(FIREWALL_CMD_PATH, port)
        output = ssr.utils.subprocess_has_output(command)
        return (output == 'yes')

    def __add_port(self, port):
        if self.__has_port(port):
            return
        command = '{0} --add-port={1} --permanent'.format(FIREWALL_CMD_PATH, port)
        ssr.utils.subprocess_not_output(command)

    def __remove_port(self, port):
        if not self.__has_port(port):
            return
        command = '{0} --remove-port={1} --permanent'.format(FIREWALL_CMD_PATH, port)
        ssr.utils.subprocess_not_output(command)

    def list_ports(self):
        command = '{0} --list-ports --permanent'.format(FIREWALL_CMD_PATH)
        return ssr.utils.subprocess_has_output(command).split()

    def set_ports(self, ports):
        old_ports = self.list_ports()
        common_ports = set(ports).intersection(set(old_ports))

        for port in ports:
            if not common_ports.__contains__(port):
                self.__add_port(port)

        for port in old_ports:
            if not common_ports.__contains__(port):
                self.__remove_port(port)

    def clear_ports(self):
        ports = self.list_ports()
        for port in ports:
            self.__remove_port(port)

    def reload(self):
        command = '{0} --reload'.format(FIREWALL_CMD_PATH)
        return ssr.utils.subprocess_not_output(command)

    def list_icmp_blocks(self):
        command = '{0} --list-icmp-blocks --permanent'.format(FIREWALL_CMD_PATH)
        return ssr.utils.subprocess_has_output(command)

    def add_icmp_blocks(self, block):
        command = '{0} --add-icmp-block={1} --permanent'.format(FIREWALL_CMD_PATH, block)
        ssr.utils.subprocess_not_output(command)

    def remove_icmp_blocks(self, block):
        command = '{0} --remove-icmp-block={1} --permanent'.format(FIREWALL_CMD_PATH, block)
        ssr.utils.subprocess_not_output(command)


# 系统防火墙服务
class Switch(Firewalld):
    def get(self):
        retdata = dict()
        retdata['enabled'] = self.firewalld_systemd.is_active()

        # 只有防火墙开启状态下才可以查询开放端口信息
        if retdata['enabled']:
            retdata['ports'] = ';'.join(self.list_ports())
        else:
            retdata['ports'] = str()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        # 也可以不用捕获异常，后台框架会对异常进行处理
        try:
            if args['enabled']:
                self.firewalld_systemd.enable()
                self.firewalld_systemd.start()
            else:
                if self.firewalld_systemd.stop():
                    return (False, 'Unable to stop firewalld service! \t\t')
                self.firewalld_systemd.disable()
        except Exception as e:
            return (False, e)

        if self.firewalld_systemd.is_active():
            if len(args['ports']) == 0:
                self.clear_ports()
            else:
                self.set_ports(args['ports'].split(';'))
            self.reload()
        return (True, '')


# ICMP时间戳请求
class IcmpTimestamp(Firewalld):
    def get(self):
        retdata = dict()
        if self.firewalld_systemd.is_active():
            command_output = self.list_icmp_blocks()
            ssr.log.debug(command_output)
            icmp_blocks = command_output.split()
            retdata['timestamp_request'] = (not icmp_blocks.__contains__('timestamp-request'))
        else:
            retdata['timestamp_request'] = True
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if self.firewalld_systemd.is_active():
            if args['timestamp_request']:
                self.remove_icmp_blocks('timestamp-request')
            else:
                self.add_icmp_blocks('timestamp-request')
            self.reload()
        else:
            if not args['timestamp_request']:
                raise Exception('FirewallD is not running')
        return (True, '')

class Traceroute(Firewalld):
    def get(self):
        retdata = dict()
        if self.firewalld_systemd.is_active():
            command_output = ssr.utils.subprocess_has_output(STATUS_TRACEROUTE_CMD)
            ssr.log.debug(command_output)
            retdata['enabled'] = len(command_output) != 0
        else:
            retdata['enabled'] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if self.firewalld_systemd.is_active():
            if args['enabled']:
                ssr.utils.subprocess_not_output(DISABLE_TRACEROUTE_CMD)
            else:
                ssr.utils.subprocess_not_output(OPEN_TRACEROUTE_CMD)
            # self.reload()
        else:
            if not args['enabled']:
                raise Exception('FirewallD is not running')
        return (True, '')
