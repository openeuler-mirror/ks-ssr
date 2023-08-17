import json
import ssr.systemd
from ssr import log

FIREWALL_CMD_PATH = '/usr/bin/firewall-cmd'


def switch_get():
    systemd_proxy = ssr.systemd.Proxy('firewalld')
    retdata = dict()
    retdata['enabled'] = systemd_proxy.is_active()
    return (True, json.dumps(retdata))


def switch_set(args_json):
    args = json.loads(args_json)
    systemd_proxy = ssr.systemd.Proxy('firewalld')

    # 也可以不用捕获异常，后台框架会对异常进行处理
    try:
        if args['enabled']:
            systemd_proxy.enable()
            systemd_proxy.start()
        else:
            systemd_proxy.disable()
            systemd_proxy.stop()
        return (True, '')
    except Exception as e:
        return (False, e)


def icmp_timestamp_get():
    retdata = dict()
    command = '{0} --list-icmp-blocks'.format(FIREWALL_CMD_PATH)
    command_output = ssr.utils.subprocess_has_output(command)
    log.debug(command_output)
    icmp_blocks = command_output.split()
    retdata['disable_timestamp_request'] = icmp_blocks.__contains__('timestamp-request')
    return (True, json.dumps(retdata))


def icmp_timestamp_set(args_json):
    args = json.loads(args_json)
    set_command = '{0} --{1}-icmp-block=timestamp-request'.format(FIREWALL_CMD_PATH, 'add' if args['disable_timestamp_request'] else 'remove')
    ssr.utils.subprocess_not_output(set_command)
    reload_command = '{0} --reload'.format(FIREWALL_CMD_PATH)
    ssr.utils.subprocess_not_output(reload_command)
    return (True, '')