reinforcements = (
    {
        'name': 'network-firewalld-switch',
        'module': 'firewalld',
        'function_prefix': 'switch'
    },
    {
        'name': 'network-firewalld-icmp-timestamp',
        'module': 'firewalld',
        'function_prefix': 'icmp_timestamp'
    },
    {
        'name': 'network-sysctl-redirect',
        'module': 'sysctl',
        'function_prefix': 'redirect'
    },
    {
        'name': 'network-sysctl-source-route',
        'module': 'sysctl',
        'function_prefix': 'source_route'
    },
)