reinforcements = (
    {
        'name': 'network-firewalld-switch',
        'module': 'firewalld',
        'class': 'FirewallManager'
    },
    {
        'name': 'network-icmp-timestamp',
        'module': 'firewalld',
        'class': 'IcmpTimestamp'
    },
    {
        'name': 'network-icmp-traceroute',
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