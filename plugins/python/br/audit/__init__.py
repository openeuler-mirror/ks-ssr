reinforcements = (
    {
        'name': 'audit-auditd-switch',
        'module': 'auditd',
        'class': 'Switch'
    }, 
    {
        'name': 'audit-auditd-rules',
        'module': 'auditd',
        'class': 'Rules'
    },
    {
        'name': 'audit-logrotate-rotate',
        'module': 'logrotate',
        'class': 'Rotate'
    },
    {
        'name': 'audit-logfile-permissions',
        'module': 'logfile',
        'class': 'Permissions'
    },
)
