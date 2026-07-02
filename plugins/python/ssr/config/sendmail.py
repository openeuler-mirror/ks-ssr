#--coding:utf8 --

from ssr.systemd import SwitchBase


class Switch(SwitchBase):
    def __init__(self):
        super(Switch, self).__init__('sendmail')
