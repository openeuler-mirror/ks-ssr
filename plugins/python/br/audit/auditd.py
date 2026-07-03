#--coding:utf8 --

from br.systemd import SwitchBase


# 系统审计服务
class Switch(SwitchBase):
    def __init__(self):
        super(Switch, self).__init__('auditd')
