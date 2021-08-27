reinforcements = (
    {
        'name': 'network-firewalld-switch',
        'module': 'firewalld',
        'class': 'Switch'
    },
    {
        'name': 'network-firewalld-icmp-timestamp',
        'module': 'firewalld',
        'class': 'IcmpTimestamp'
    },
    {
        'name': 'network-sysctl-redirect',
        'module': 'sysctl',
        'class': 'Redirect'
    },
    {
        'name': 'network-sysctl-source-route',
        'module': 'sysctl',
        'class': 'SourceRoute'
    },
)