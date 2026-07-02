import json
import ssr.config

SYSCTL_PATH = '/usr/sbin/sysctl'

SYSCTL_CONFI_FILE = "/etc/sysctl.d/10-sysctl-ssr.conf"
SYSCTL_CONFIG_FIELD_PARTTERN = "\\s*=\\s*"
SYSCTL_ACCEPT_REDIRECTS_PATTERN = "net.ipv4.conf.*.accept_redirects"
SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN = "net.ipv4.conf.*.accept_source_route"


def sysctl_get_items_by_pattern(partten):
    output = ssr.utils.subprocess_has_output('{0} -a -r {1}'.format(SYSCTL_PATH, partten))
    lines = output.split('\n')
    retval = list()
    for line in lines:
        fields = line.split()
        if len(fields) != 2:
            continue
        retval.append((fields[0], fields[1]))
    return retval


def redirect_get():
    retdata = dict()
    redirect_items = sysctl_get_items_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN)

    enabled = False
    for redirect_item in redirect_items:
        if redirect_item[1] == "1":
            enabled = True
            break

    retdata['enabled'] = enabled
    return (True, json.dumps(retdata))


def redirect_set(args_json):
    args = json.loads(args_json)
    redirect_items = sysctl_get_items_by_pattern(SYSCTL_ACCEPT_REDIRECTS_PATTERN)
    sysctl_config = ssr.config.Plain(SYSCTL_CONFI_FILE, SYSCTL_CONFIG_FIELD_PARTTERN)
    enabled = args['enabled']

    for redirect_item in redirect_items:
        sysctl_config.set_value(redirect_item, "1" if enabled else "0")

    # 从文件中刷新
    ssr.utils.subprocess_not_output('{0} --load'.format(SYSCTL_PATH))
    return (True, '')


def source_route_get():
    retdata = dict()
    redirect_items = sysctl_get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)

    enabled = False
    for redirect_item in redirect_items:
        if redirect_item[1] == "1":
            enabled = True
            break

    retdata['enabled'] = enabled
    return (True, json.dumps(retdata))


def source_route_set(args_json):
    args = json.loads(args_json)
    redirect_items = sysctl_get_items_by_pattern(SYSCTL_ACCEPT_SOURCE_ROUTE_PATTERN)
    sysctl_config = ssr.config.Plain(SYSCTL_CONFI_FILE)
    enabled = args['enabled']

    for redirect_item in redirect_items:
        sysctl_config.set_value(redirect_item, "1" if enabled else "0")

    # 从文件中刷新
    ssr.utils.subprocess_not_output('{0} --load'.format(SYSCTL_PATH))
    return (True, '')