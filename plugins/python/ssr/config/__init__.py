reinforcements = (
    {
        'name': 'config-sendmail-switch',
        'module': 'sendmail',
        'class': 'Switch'
    },
    {
        'name': 'config-history-size',
        'module': 'resource',
        'class': 'HistorySizeLimit'
    },
    {
        'name': 'config-login-lock',
        'module': 'login-lock',
        'class': 'LoginLock'
    },
    {
        'name': 'config-password-expired',
        'module': 'password',
        'class': 'PasswordExpired'
    },
    {
        'name': 'config-password-complexity',
        'module': 'password',
        'class': 'PasswordComplexity'
    },
    {
        'name': 'config-file-permissions',
        'module': 'permissions',
        'class': 'PermissionSetting'
    },
    {
        'name': 'config-directory-permissions',
        'module': 'permissions',
        'class': 'DirectoryPermissionSetting'
    },
    {
        'name': 'config-key-reboot',
        'module': 'sysctl',
        'class': 'KeyRebootSwitch'
    },
    {
        'name': 'config-resource-limits',
        'module': 'resource',
        'class': 'ResourceLimits'
    },
    {
        'name': 'config-sak-key',
        'module': 'sysctl',
        'class': 'SAKKey'
    },
    {
        'name': 'config-dmesg',
        'module': 'sysctl',
        'class': 'Dmesg'
    },
    {
        'name': 'config-umask-limit',
        'module': 'permissions',
        'class': 'UmaskLimit'
    },
    {
        'name': 'config-nouser-files',
        'module': 'scan-files',
        'class': 'NouserFiles'
    },
    {
        'name': 'config-authority-files',
        'module': 'scan-files',
        'class': 'AuthorityFiles'
    },
    {
        'name': 'config-suid-sgid-files',
        'module': 'scan-files',
        'class': 'SuidSgidFiles'
    },
    {
        'name': 'config-vulnerability-scanning',
        'module': 'vulnerability',
        'class': 'VulnerabilityScan'
    },
    {
        'name': 'config-resource-monitor',
        'module': 'resource',
        'class': 'ResourceMonitor'
    }
)