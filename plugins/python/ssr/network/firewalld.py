#--coding:utf8 --

import json
import ssr.systemd
import ssr.log

FIREWALL_CMD_PATH = '/usr/bin/firewall-cmd'

FIREWALL_ADD_RULE_CMD = 'firewall-cmd --direct --permanent --add-rule'
FIREWALL_REMOVE_RULE_CMD = 'firewall-cmd --direct --permanent --remove-rule'
FIREWALL_FIND_RULE_CMD = 'firewall-cmd --direct --get-all-rules'

#iptables -w参数防止进程抢占

IPTABLES_LIMITS_PORTS = "21,25,110,137,138,67,68,161,162,139,389,873,445,631"

# iptables INPUT icmp  DROP
IPTABLES_INPUT_ICMP = "ipv4 filter INPUT 0 -p ICMP --icmp-type"

# iptables OUTPUT icmp  DROP
IPTABLES_OUTPUT_ICMP = "ipv4 filter OUTPUT 0 -p ICMP --icmp-type"

# iptables INPUT tcp 
IPTABLES_INPUT_TCP = "ipv4 filter INPUT 0 -p tcp"

# iptables OUTPUT tcp 
IPTABLES_OUTPUT_TCP = "ipv4 filter OUTPUT 0 -p tcp"

# iptables INPUT udp 
IPTABLES_INPUT_UDP = "ipv4 filter INPUT 0 -p udp"

# iptables OUTPUT udp 
IPTABLES_OUTPUT_UDP = "ipv4 filter INPUT 0 -p udp"

# 清空链规则； 清空子链规则
CLEAR_IPTABLES = "iptables -F; iptables -X"
CLEAR_IPTABLES_INPUT = "iptables -F INPUT"
CLEAR_IPTABLES_OUTPUT = "iptables -F OUTPUT"

FIND_IPTABLES_INPUT = "iptables -L"

# 禁止主机被Traceroute检测 time-exceeded
TRACEROUTE_DETAIL = "time-exceeded -j REJECT"

# ICMP时间戳请求 time-exceeded
TIMESTAMP_REQUEST_DETAIL = "timestamp-request -j REJECT"
TIMESTAMP_REPLY_DETAIL = "timestamp-reply -j REJECT"

class Firewall:
    def __init__(self):
        self.firewalld_systemd = ssr.systemd.Proxy('firewalld')

    def reload(self):
        command = '{0} --reload'.format(FIREWALL_CMD_PATH)
        return ssr.utils.subprocess_not_output(command)

    # target_cmd 为筛选关键字命令
    def del_iptables_history(self,target_cmd):
        output_result = str(ssr.utils.subprocess_has_output(FIREWALL_FIND_RULE_CMD + " {0}".format(target_cmd)))
        for line in output_result.splitlines():
            ssr.log.debug("output_result.splitlines() = ",line)
            ssr.utils.subprocess_not_output(FIREWALL_REMOVE_RULE_CMD + " {0}".format(line))

# 系统防火墙服务
class Switch(Firewall):
    def get(self):
        retdata = dict()
        retdata['enabled'] = self.firewalld_systemd.is_active()
        retdata['threat-port'] = len(ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} --dport 21 -j REJECT'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_INPUT_TCP))) != 0 or len(ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} --dport 21 -j REJECT'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_INPUT_UDP))) != 0
        # 设置tcp或udp都符合
        retdata['tcp-udp'] = "tcp"
        # 是否清空规则与最大连接数由用户决定，默认为否,都为符合
        retdata['clear-configuration'] = False
        retdata['input-ports-connect-nums'] = 0
        
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

        # ssr.utils.subprocess_not_output(OPEN_IPTABLES_ALL_PORTS)
        if self.firewalld_systemd.is_active():
            if args['input-ports-connect-nums']  == 0:
                self.del_iptables_history('|grep 1:60999')
            else:
                self.del_iptables_history('|grep 1:60999')
                ssr.utils.subprocess_not_output("{0} {1} --dport 1:60999 -m connlimit --connlimit-above {2} --connlimit-mask 0 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_TCP, args['input-ports-connect-nums']))
                ssr.utils.subprocess_not_output("{0} {1} --dport 1:60999 -m connlimit --connlimit-above {2} --connlimit-mask 0 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_UDP, args['input-ports-connect-nums']))

            self.reload()
    
            # Disable FTP, SMTP and other ports that may threaten the system
            if args['threat-port']:
                for port in IPTABLES_LIMITS_PORTS.split(","):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_not_output("{0} {1} --dport {2} -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_TCP, port))
                        ssr.utils.subprocess_not_output("{0} {1} --dport {2} -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_TCP, port))
                    else:
                        ssr.utils.subprocess_not_output("{0} {1} --dport {2} -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_UDP, port))
                        ssr.utils.subprocess_not_output("{0} {1} --dport {2} -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_UDP, port))
            else:
                for port in IPTABLES_LIMITS_PORTS.split(","):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_has_output_ignore_error_handling("{0} {1} --dport {2} -j REJECT".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_INPUT_TCP, port))
                        ssr.utils.subprocess_has_output_ignore_error_handling("{0} {1} --dport {2} -j REJECT".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_OUTPUT_TCP, port))
                    else:
                        ssr.utils.subprocess_has_output_ignore_error_handling("{0} {1} --dport {2} -j REJECT".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_INPUT_UDP, port))
                        ssr.utils.subprocess_has_output_ignore_error_handling("{0} {1} --dport {2} -j REJECT".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_OUTPUT_UDP, port))
            self.reload()

            # 禁用网段
            if len(args['disable-network-segment']) != 0:
                self.del_iptables_history("|grep OUTPUT |grep -E '/[1-9][0-2]|/[1-9]' |grep -v 1:60999")
                for network_segment in args['disable-network-segment'].split(","):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_has_output("{0} {1} -s {2} --dport 1:1023 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_TCP, network_segment))
                    else:
                        ssr.utils.subprocess_has_output("{0} {1} -s {2} --dport 1:1023 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_UDP, network_segment))
            else:
                self.del_iptables_history("|grep -E '/[1-9][0-2]|/[1-9]' |grep -v 1:60999")
            self.reload()
            
            # 禁用端口
            if len(args['disable-ports']) != 0:
                self.del_iptables_history("|grep INPUT |grep dport |grep -v -E '{0}' ".format(IPTABLES_LIMITS_PORTS.replace(",","|")))
                for port in args['disable-ports'].split(";"):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_has_output("{0} {1} --dport {2}  -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_TCP, port))
                    else:
                        ssr.utils.subprocess_has_output("{0} {1} --dport {2}  -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_UDP, port))
            else:
                self.del_iptables_history("|grep INPUT |grep dport |grep -v -E '{0}' ".format(IPTABLES_LIMITS_PORTS.replace(",","|")))
            self.reload()

            # 禁用网段 output
            if len(args['disable-network-segment-output']) != 0:
                self.del_iptables_history("|grep OUTPUT |grep -E '/[1-9][0-2]|/[1-9]' |grep -v 1:60999")
                for network_segment in args['disable-network-segment-output'].split(","):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_has_output("{0} {1} -d {2} --dport 1:1023 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_TCP, network_segment))
                    else:
                        ssr.utils.subprocess_has_output("{0} {1} -d {2} --dport 1:1023 -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_UDP, network_segment))
            else:
                self.del_iptables_history("|grep OUTPUT |grep -E '/[1-9][0-2]|/[1-9]' |grep -v 1:60999")
            self.reload()

            # 禁用端口 output
            if len(args['disable-ports-output']) != 0:
                self.del_iptables_history("|grep OUTPUT |grep dport |grep -v -E '{0}' ".format(IPTABLES_LIMITS_PORTS.replace(",","|")))
                for port in args['disable-ports-output'].split(";"):
                    if args['tcp-udp'] == "tcp":
                        ssr.utils.subprocess_has_output("{0} {1} --dport {2}  -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_TCP, port))
                    else:
                        ssr.utils.subprocess_has_output("{0} {1} --dport {2}  -j REJECT".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_UDP, port))
            else:
                self.del_iptables_history("|grep OUTPUT |grep dport |grep -v -E '{0}' ".format(IPTABLES_LIMITS_PORTS.replace(",","|")))
            self.reload()

            if args['clear-configuration']:
                self.del_iptables_history("")
            self.reload()
            
        return (True, '')


# ICMP时间戳请求
class IcmpTimestamp(Firewall):
    def get(self):
        retdata = dict()
        iptable_input = ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} {2}'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REQUEST_DETAIL))
        iptable_output = ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} {2}'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REPLY_DETAIL))
        # ssr.log.debug(command_output)
        if len(iptable_output) == 0 and len(iptable_input) == 0:
            retdata['timestamp_request'] = True
        else:
            retdata['timestamp_request'] = False
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if self.firewalld_systemd.is_active():
            if args['timestamp_request']:
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REQUEST_DETAIL))
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REPLY_DETAIL))
            else:
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REQUEST_DETAIL))
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_ICMP, TIMESTAMP_REPLY_DETAIL))
            self.reload()
        else:
            if not args['timestamp_request']:
                raise Exception('Firewalld is not running!')
        return (True, '')

# 禁止主机被Traceroute检测
class Traceroute(Firewall):
    def get(self):
        retdata = dict()
        iptable_input = ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} {2}'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_INPUT_ICMP, TRACEROUTE_DETAIL))
        iptable_output = ssr.utils.subprocess_has_output_ignore_error_handling("{0} |grep '{1} {2}'".format(FIREWALL_FIND_RULE_CMD, IPTABLES_OUTPUT_ICMP, TRACEROUTE_DETAIL))
        # ssr.log.debug(command_output)
        retdata['enabled'] = len(iptable_output) != 0 and len(iptable_input) != 0
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if self.firewalld_systemd.is_active():
            if args['enabled']:
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_ADD_RULE_CMD, IPTABLES_INPUT_ICMP, TRACEROUTE_DETAIL))
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_ADD_RULE_CMD, IPTABLES_OUTPUT_ICMP, TRACEROUTE_DETAIL))
            else:
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_INPUT_ICMP, TRACEROUTE_DETAIL))
                ssr.utils.subprocess_not_output("{0} {1} {2}".format(FIREWALL_REMOVE_RULE_CMD, IPTABLES_OUTPUT_ICMP, TRACEROUTE_DETAIL))
            self.reload()
        else:
            if not args['enabled']:
                raise Exception('Firewalld is not running!')

        return (True, '')
