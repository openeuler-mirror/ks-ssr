# -*- coding: utf-8 -*-

import json
import br.systemd
import br.log

FIREWALL_CMD_PATH = '/usr/bin/firewall-cmd'
IPTABLES_RULES_SAVE_PATH = '/usr/share/ks-ssr/br-iptables.rules'

# iptables -w参数防止进程抢占

# 开放443端口(HTTPS)
OPEN_IPTABLES_PORTS = 'iptables -w 0.05 -I INPUT -p tcp --dport 443 -j ACCEPT'
CHECK_IPTABLES_PORTS = 'iptables -w 0.05 -C INPUT -p tcp --dport 443 -j ACCEPT'
CLOSE_IPTABLES_PORTS = 'iptables -w 0.05 -D INPUT -p tcp --dport 443 -j ACCEPT'

IPTABLES_LIMITS_PORTS = "21,25,110,137,138,67,68,161,162,139,389,873,445,631"

# iptables INPUT icmp  DROP
DISABLE_IPTABLES_INPUT_ICMP = "iptables -w 0.05 -I INPUT -p ICMP --icmp-type"
CHECK_IPTABLES_INPUT_ICMP = "iptables -w 0.05 -C INPUT -p ICMP --icmp-type"
OPEN_IPTABLES_INPUT_ICMP = "iptables -w 0.05 -D INPUT -p ICMP --icmp-type"
# iptables OUTPUT icmp  DROP
DISABLE_IPTABLES_OUTPUT_ICMP = "iptables -w 0.05 -I OUTPUT -p ICMP --icmp-type"
CHECK_IPTABLES_OUTPUT_ICMP = "iptables -w 0.05 -C OUTPUT -p ICMP --icmp-type"
OPEN_IPTABLES_OUTPUT_ICMP = "iptables -w 0.05 -D OUTPUT -p ICMP --icmp-type"
# iptables INPUT tcp
ADD_IPTABLES_INPUT_TCP = "iptables -w 0.05 -I INPUT -p tcp"
CHECK_IPTABLES_INPUT_TCP = "iptables -w 0.05 -C INPUT -p tcp"
DELETE_IPTABLES_INPUT_TCP = "iptables -w 0.05 -D INPUT -p tcp"
# iptables OUTPUT tcp
ADD_IPTABLES_OUTPUT_TCP = "iptables -w 0.05 -I OUTPUT -p tcp"
CHECK_IPTABLES_OUTPUT_TCP = "iptables -w 0.05 -C OUTPUT -p tcp"
DELETE_IPTABLES_OUTPUT_TCP = "iptables -w 0.05 -D OUTPUT -p tcp"

# iptables INPUT udp
ADD_IPTABLES_INPUT_UDP = "iptables -w 0.05 -I INPUT -p udp"
CHECK_IPTABLES_INPUT_UDP = "iptables -w 0.05 -C INPUT -p udp"
DELETE_IPTABLES_INPUT_UDP = "iptables -w 0.05 -D INPUT -p udp"
# iptables OUTPUT udp
ADD_IPTABLES_OUTPUT_UDP = "iptables -w 0.05 -I OUTPUT -p udp"
CHECK_IPTABLES_OUTPUT_UDP = "iptables -w 0.05 -C OUTPUT -p udp"
DELETE_IPTABLES_OUTPUT_UDP = "iptables -w 0.05 -D OUTPUT -p udp"

# 清空链规则； 清空子链规则
CLEAR_IPTABLES = "iptables -F; iptables -X"
CLEAR_IPTABLES_INPUT = "iptables -F INPUT"
CLEAR_IPTABLES_OUTPUT = "iptables -F OUTPUT"

FIND_IPTABLES_INPUT = "iptables -L"
FIND_IPTABLES_TAIL = "-n --line-numbers"
IPTABLES_REJECT_TAIL = "-j REJECT"
IPTABLES_DPORTS = "--dport "
IPTABLES_DPORTS_CHECK_CMD = "dpt: |grep REJECT |grep -v -E"
IPTABLES_SEGMENT_CHECK = "-E '/[1-9][0-2]|/[1-9]' |grep -v 1:60999"
IPTABLES_SEGMENT_REJECT_TAIL = "--dport 1:1023 -j REJECT"

# 禁止主机被Traceroute检测 time-exceeded
TRACEROUTE_DETAIL = "time-exceeded -j DROP"

# ICMP时间戳请求 time-exceeded
TIMESTAMP_REQUEST_DETAIL = "timestamp-request -j DROP"
TIMESTAMP_REPLY_DETAIL = "timestamp-reply -j DROP"

# ICMP echo-request (ping)
TIMESTAMP_ECHO_REQUEST = "echo-request -j REJECT"

# 开放所有端口
OPEN_IPTABLES_ALL_PORTS = "iptables -P INPUT ACCEPT; iptables -P OUTPUT ACCEPT; iptables -P FORWARD ACCEPT"

ERROR_NOTIFY = "Unable to stop firewalld service!"


class Firewall:
    def __init__(self):
        self.firewalld_systemd = br.systemd.Proxy('firewalld')
        self.iptables_systemd = br.systemd.Proxy('iptables')
        self.flag_first = True
        br.utils.subprocess_not_output(
            "iptables-restore < {0}".format(IPTABLES_RULES_SAVE_PATH))

    def save_iptables_rule(self):
        br.utils.subprocess_not_output(
            "iptables-save > {0}".format(IPTABLES_RULES_SAVE_PATH))

    def restore_iptables_rule(self):
        br.utils.subprocess_not_output(
            "iptables-restore < {0}".format(IPTABLES_RULES_SAVE_PATH))

    def open_iptables(self):
        if self.firewalld_systemd.exist():
            if self.firewalld_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.firewalld_systemd.disable()
            self.firewalld_systemd.mask()

        if self.iptables_systemd.exist():
            self.iptables_systemd.unmask()
            self.iptables_systemd.enable()
            if len(br.utils.subprocess_has_output_ignore_error_handling(CHECK_IPTABLES_PORTS)) != 0:
                br.utils.subprocess_not_output(OPEN_IPTABLES_PORTS)
            self.iptables_systemd.start()
            self.iptables_systemd.service_save()

    def open_firewalld(self):
        if self.iptables_systemd.exist():
            if len(br.utils.subprocess_has_output_ignore_error_handling(CHECK_IPTABLES_PORTS)) == 0:
                br.utils.subprocess_not_output(CLOSE_IPTABLES_PORTS)
            if self.iptables_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.iptables_systemd.disable()
            self.iptables_systemd.service_save()
            self.iptables_systemd.mask()

        if self.firewalld_systemd.exist():
            self.firewalld_systemd.unmask()
            self.firewalld_systemd.enable()
            self.firewalld_systemd.start()

    def close_iptables(self):
        if self.iptables_systemd.exist():
            if len(br.utils.subprocess_has_output_ignore_error_handling(CHECK_IPTABLES_PORTS)) == 0:
                br.utils.subprocess_not_output(CLOSE_IPTABLES_PORTS)
            if self.iptables_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.iptables_systemd.disable()
            self.iptables_systemd.service_save()
            # self.iptables_systemd.mask()

    def set_iptables(self, iptables_set_cmd, iptables_check_cmd, set_name, set_type):
        set_cmd = '{0}  {1}  {2}'.format(iptables_set_cmd, set_name, set_type)
        check_cmd = '{0}  {1}  {2}'.format(
            iptables_check_cmd, set_name, set_type)
        br.log.debug(
            br.utils.subprocess_has_output_ignore_error_handling(check_cmd))
        if len(br.utils.subprocess_has_output_ignore_error_handling(check_cmd)) != 0:
            br.utils.subprocess_not_output(set_cmd)

    def del_iptables(self, iptables_del_cmd, iptables_check_cmd, set_name, set_type):
        set_cmd = '{0}  {1}  {2}'.format(iptables_del_cmd, set_name, set_type)
        check_cmd = '{0}  {1}  {2}'.format(
            iptables_check_cmd, set_name, set_type)
        br.log.debug(
            br.utils.subprocess_has_output_ignore_error_handling(check_cmd))
        if len(br.utils.subprocess_has_output_ignore_error_handling(check_cmd)) == 0:
            br.utils.subprocess_not_output(set_cmd)

    def del_iptables_history(self, target, iptablestype="INPUT"):
        count = 0
        grep_cmd = " |grep " + target + " "
        output_result = str(br.utils.subprocess_has_output(
            FIND_IPTABLES_INPUT + " {0} {1}".format(iptablestype, FIND_IPTABLES_TAIL) + grep_cmd))
        for line in output_result.splitlines():
            br.log.debug("output_result.splitlines() = ", line)
            count = count + 1
        for i in range(count):
            br.log.debug("index = ", i, str(br.utils.subprocess_has_output(
                FIND_IPTABLES_INPUT + " {0} {1}".format(iptablestype, FIND_IPTABLES_TAIL) + grep_cmd)).splitlines()[0].split(' ')[0])
            br.utils.subprocess_not_output('iptables -D {0} {1}'.format(iptablestype, str(br.utils.subprocess_has_output(
                FIND_IPTABLES_INPUT + " {0} {1}".format(iptablestype, FIND_IPTABLES_TAIL) + grep_cmd).splitlines()[0].split(' ')[0])))


# 系统防火墙服务
class FirewallManager(Firewall):
    def disable_firewalld_service(self):
        if self.firewalld_systemd.is_active():
            if self.firewalld_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.firewalld_systemd.disable()
    def clean_rule(self):
        if self.flag_first:
            br.utils.subprocess_not_output(CLEAR_IPTABLES)
            br.utils.subprocess_not_output(OPEN_IPTABLES_ALL_PORTS)
            self.flag_first = False
    def set_input_ports_connect_num(self, arg):
        if arg == 0:
            self.del_iptables_history('1:60999')
        else:
            self.del_iptables_history('1:60999')
            self.set_iptables(ADD_IPTABLES_INPUT_TCP, CHECK_IPTABLES_INPUT_TCP,
                              "--dport 1:60999 -m connlimit --connlimit-above {0} --connlimit-mask 0".format(arg), IPTABLES_REJECT_TAIL)
            self.set_iptables(ADD_IPTABLES_INPUT_UDP, CHECK_IPTABLES_INPUT_UDP,
                              "--dport 1:60999 -m connlimit --connlimit-above {0} --connlimit-mask 0".format(arg), IPTABLES_REJECT_TAIL)
    def disable_threat_port(self, network_type):
        for port in IPTABLES_LIMITS_PORTS.split(","):
            if "tcp" in network_type:
                self.set_iptables(
                    ADD_IPTABLES_INPUT_TCP, CHECK_IPTABLES_INPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                self.set_iptables(
                    ADD_IPTABLES_OUTPUT_TCP, CHECK_IPTABLES_OUTPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
            else:
                self.set_iptables(
                    ADD_IPTABLES_INPUT_UDP, CHECK_IPTABLES_INPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                self.set_iptables(
                    ADD_IPTABLES_OUTPUT_UDP, CHECK_IPTABLES_OUTPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
    def enable_threat_port(self, network_type):
        for port in IPTABLES_LIMITS_PORTS.split(","):
            if "tcp" in network_type:
                self.del_iptables(
                    DELETE_IPTABLES_INPUT_TCP, CHECK_IPTABLES_INPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                self.del_iptables(
                    DELETE_IPTABLES_OUTPUT_TCP, CHECK_IPTABLES_OUTPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
            else:
                self.del_iptables(
                    DELETE_IPTABLES_INPUT_UDP, CHECK_IPTABLES_INPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                self.del_iptables(
                    DELETE_IPTABLES_OUTPUT_UDP, CHECK_IPTABLES_OUTPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)

    def set_network_segments(self, network_segments, network_type):
        if len(network_segments.replace('"', '')) != 0:
            self.del_iptables_history(
                IPTABLES_SEGMENT_CHECK)
            for network_segment in network_segments.split(","):
                if "tcp" in network_type:
                    self.set_iptables(ADD_IPTABLES_INPUT_TCP, CHECK_IPTABLES_INPUT_TCP,
                                      "-s " + network_segment, IPTABLES_SEGMENT_REJECT_TAIL)
                else:
                    self.set_iptables(ADD_IPTABLES_INPUT_UDP, CHECK_IPTABLES_INPUT_UDP,
                                      "-s " + network_segment, IPTABLES_SEGMENT_REJECT_TAIL)
        else:
            self.del_iptables_history(IPTABLES_SEGMENT_CHECK)
    def set_network_segments_output(self, network_segments, network_type):
        if len(network_segments.replace('"', '')) != 0:
            self.del_iptables_history(
                IPTABLES_SEGMENT_CHECK, "OUTPUT")
            for network_segment in network_segments.split(","):
                if "tcp" in network_type:
                    self.set_iptables(ADD_IPTABLES_OUTPUT_TCP, CHECK_IPTABLES_OUTPUT_TCP,
                                      "-d " + network_segment, IPTABLES_SEGMENT_REJECT_TAIL)
                else:
                    self.set_iptables(ADD_IPTABLES_OUTPUT_UDP, CHECK_IPTABLES_OUTPUT_UDP,
                                      "-d " + network_segment, IPTABLES_SEGMENT_REJECT_TAIL)
        else:
            self.del_iptables_history(
                IPTABLES_SEGMENT_CHECK, "OUTPUT")

    def set_ports(self, ports, dpt_str, network_type):
        marks = "'"
        if len(ports.replace('"', '')) != 0:
            self.del_iptables_history("{0} {1}{2}{3} ".format(
                IPTABLES_DPORTS_CHECK_CMD, marks, "dpt:" + IPTABLES_LIMITS_PORTS.replace(",", dpt_str), marks))
            for port in ports.split(";"):
                if "tcp" in network_type:
                    self.set_iptables(
                        ADD_IPTABLES_INPUT_TCP, CHECK_IPTABLES_INPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                else:
                    self.set_iptables(
                        ADD_IPTABLES_INPUT_UDP, CHECK_IPTABLES_INPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
        else:
            self.del_iptables_history("{0} {1}{2}{3} ".format(
                IPTABLES_DPORTS_CHECK_CMD, marks, "dpt:" + IPTABLES_LIMITS_PORTS.replace(",", dpt_str), marks))
    def set_ports_output(self, ports, dpt_str, network_type):
        marks = "'"
        if len(ports.replace('"', '')) != 0:
            self.del_iptables_history("{0} {1}{2}{3} ".format(
                IPTABLES_DPORTS_CHECK_CMD, marks, "dpt:" + IPTABLES_LIMITS_PORTS.replace(",", dpt_str), marks), "OUTPUT")
            for port in ports.split(";"):
                if "tcp" in network_type:
                    self.set_iptables(
                        ADD_IPTABLES_OUTPUT_TCP, CHECK_IPTABLES_OUTPUT_TCP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
                else:
                    self.set_iptables(
                        ADD_IPTABLES_OUTPUT_UDP, CHECK_IPTABLES_OUTPUT_UDP, IPTABLES_DPORTS + port, IPTABLES_REJECT_TAIL)
        else:
            self.del_iptables_history("{0} {1}{2}{3} ".format(
                IPTABLES_DPORTS_CHECK_CMD, marks, "dpt:" + IPTABLES_LIMITS_PORTS.replace(",", dpt_str), marks), "OUTPUT")

    def get(self):
        retdata = dict()
        retdata['threat-port'] = len(br.utils.subprocess_has_output_ignore_error_handling(CHECK_IPTABLES_INPUT_TCP + " --dport 21 -j REJECT")
                                     ) == 0 or len(br.utils.subprocess_has_output_ignore_error_handling(CHECK_IPTABLES_INPUT_UDP + " --dport 21 -j REJECT")) == 0
        # 设置tcp或udp都符合
        retdata['tcp-udp'] = "tcp"
        # 是否清空规则与最大连接数由用户决定，默认为否,都为符合
        retdata['clear-configuration'] = False
        retdata['input-ports-connect-nums'] = 0
        retdata['disable-ping'] = len(br.utils.subprocess_has_output_ignore_error_handling(
            CHECK_IPTABLES_INPUT_ICMP + " " + TIMESTAMP_ECHO_REQUEST)) == 0

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        dpt_str = "|dpt:"
        network_type = args['tcp-udp']
        self.disable_firewalld_service()
        self.clean_rule()

        if "tcp" in network_type:
            self.del_iptables_history('udp', 'INPUT')
            self.del_iptables_history('udp', 'OUTPUT')
        else:
            self.del_iptables_history('tcp', 'INPUT')
            self.del_iptables_history('tcp', 'OUTPUT')

        self.set_input_ports_connect_num(args['input-ports-connect-nums'])

        # Disable FTP, SMTP and other ports that may threaten the system
        if args['threat-port']:
            self.disable_threat_port(network_type)
        else:
            self.enable_threat_port(network_type)

        # 禁用网段
        self.set_network_segments(args['disable-network-segment'], network_type)
        # 禁用端口
        self.set_ports(args['disable-ports'], dpt_str, network_type)
        # 禁用网段 output
        self.set_network_segments_output(args['disable-network-segment-output'], network_type)
        # 禁用端口 output
        self.set_ports_output(args['disable-ports-output'], dpt_str, network_type)

        if args['disable-ping']:
            self.set_iptables(DISABLE_IPTABLES_INPUT_ICMP, CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_ECHO_REQUEST, "")
        else:
            self.del_iptables(OPEN_IPTABLES_INPUT_ICMP, CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_ECHO_REQUEST, "")

        if args['clear-configuration']:
            br.utils.subprocess_not_output(OPEN_IPTABLES_ALL_PORTS)
            br.utils.subprocess_not_output(CLEAR_IPTABLES)
            br.utils.subprocess_not_output(OPEN_IPTABLES_ALL_PORTS)

        # 保存iptables配置
        self.save_iptables_rule()

        return (True, '')


# ICMP时间戳请求
class IcmpTimestamp(Firewall):
    def get(self):
        retdata = dict()
        iptable_input = br.utils.subprocess_has_output_ignore_error_handling(
            CHECK_IPTABLES_INPUT_ICMP + " " + TIMESTAMP_REQUEST_DETAIL)
        iptable_output = br.utils.subprocess_has_output_ignore_error_handling(
            CHECK_IPTABLES_INPUT_ICMP + " " + TIMESTAMP_REPLY_DETAIL)
        if len(iptable_output) == 0 and len(iptable_input) == 0:
            retdata['timestamp_request'] = False
        else:
            retdata['timestamp_request'] = True
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if self.firewalld_systemd.is_active():
            if self.firewalld_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.firewalld_systemd.disable()

        if args['timestamp_request']:
            self.del_iptables(OPEN_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_REQUEST_DETAIL, "")
            self.del_iptables(OPEN_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_REPLY_DETAIL, "")
        else:
            self.set_iptables(DISABLE_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_REQUEST_DETAIL, "")
            self.set_iptables(DISABLE_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TIMESTAMP_REPLY_DETAIL, "")

        # 保存iptables配置
        self.save_iptables_rule()

        return (True, '')

# 禁止主机被Traceroute检测


class Traceroute(Firewall):
    def get(self):
        retdata = dict()
        iptable_input = br.utils.subprocess_has_output_ignore_error_handling(
            CHECK_IPTABLES_INPUT_ICMP + " " + TRACEROUTE_DETAIL)
        iptable_output = br.utils.subprocess_has_output_ignore_error_handling(
            CHECK_IPTABLES_OUTPUT_ICMP + " " + TRACEROUTE_DETAIL)
        # br.log.debug(command_output)
        retdata['enabled'] = len(
            iptable_output) == 0 and len(iptable_input) == 0
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if self.firewalld_systemd.is_active():
            if self.firewalld_systemd.stop():
                return (False, ERROR_NOTIFY)
            self.firewalld_systemd.disable()

        if args['enabled']:
            self.set_iptables(DISABLE_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TRACEROUTE_DETAIL, "")
            self.set_iptables(DISABLE_IPTABLES_OUTPUT_ICMP,
                              CHECK_IPTABLES_OUTPUT_ICMP, TRACEROUTE_DETAIL, "")
        else:
            self.del_iptables(OPEN_IPTABLES_INPUT_ICMP,
                              CHECK_IPTABLES_INPUT_ICMP, TRACEROUTE_DETAIL, "")
            self.del_iptables(OPEN_IPTABLES_OUTPUT_ICMP,
                              CHECK_IPTABLES_OUTPUT_ICMP, TRACEROUTE_DETAIL, "")

        # 保存iptables配置
        self.save_iptables_rule()

        return (True, '')
