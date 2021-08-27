import json
import ssr.systemd
from ssr import log


def switch_get():
    retdata = dict()
    systemd_proxy = ssr.systemd.Proxy('auditd')
    retdata['enabled'] = systemd_proxy.is_active()
    return (True, json.dumps(retdata))


def switch_set(args_json):
    args = json.loads(args_json)
    systemd_proxy = ssr.systemd.Proxy('auditd')

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