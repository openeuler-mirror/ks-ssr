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
        'name': 'network-firewalld-icmp-traceroute',
        'module': 'firewalld',
        'class': 'Traceroute'
    },
    {
        'name': 'network-sysctl-icmp-redirect',
        'module': 'sysctl',
        'class': 'IcmpRedirect'
    },
    {
        'name': 'network-sysctl-syn-flood',
        'module': 'sysctl',
        'class': 'SynFlood'
    },
    {
        'name': 'network-sysctl-source-route',
        'module': 'sysctl',
        'class': 'SourceRoute'
    },
    {
        'name': 'network-sysctl-high-risk-vulnerability',
        'module': 'vulnerability',
        'class': 'VulnerabilitySysctl'
    },
)