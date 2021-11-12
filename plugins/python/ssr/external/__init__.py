reinforcements = (
    {
        'name': 'external-udev-cdrom',
        'module': 'udev',
        'class': 'CDROM'
    },
    {
        'name': 'external-udev-usb',
        'module': 'udev',
        'class': 'USB'
    },
    {
        'name': 'external-udev-ttys',
        'module': 'udev',
        'class': 'TTYS'
    },
    {
        'name': 'external-accounts-login-limit',
        'module': 'accounts',
        'class': 'LoginLimit'
    },
    {
        'name': 'external-accounts-null-password',
        'module': 'accounts',
        'class': 'NullPassword'
    },
    {
        'name': 'external-sshd-root-login',
        'module': 'sshd',
        'class': 'RootLogin'
    },
    {
        'name': 'external-sshd-pubkey-auth',
        'module': 'sshd',
        'class': 'PubkeyAuth'
    },
    {
        'name': 'external-sshd-weak-encryption',
        'module': 'sshd',
        'class': 'WeakEncryption'
    },
    {
        'name': 'external-pam-su-wheel',
        'module': 'pam',
        'class': 'SuWheel'
    },
    {
        'name': 'external-radio-switch',
        'module': 'nm',
        'class': 'Switch'
    },
)