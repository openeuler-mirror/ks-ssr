# -*- coding: utf-8 -*-

from br.service_manager import SwitchBase
from br.service_manager import SERVICE_AUTOSTART
import json
import os
import re
import br.configuration
import br.log
import br.utils

AUDIT_RULES_PATH = "/etc/audit/rules.d/br-audit.rules"
AUDIT_WATCH_PATH_KEY = "watch-path"


# 系统审计服务
class Switch(SwitchBase):

    def __init__(self):
        super(Switch, self).__init__("auditd")

    # 特例化回滚函数是因为 auditd 服务启动后无法通过 systemctl 停止， 只能通过 service 来 停止。
    # 所以无论当前环境的服务管理命令是什么，都使用 service 来停止 auditd。
    def rollback(self, args_json):
        args = json.loads(args_json)
        try:
            if args[self.key]:
                if self.systemd_proxy.exist():
                    self.systemd_proxy.start()
            else:
                if self.systemd_proxy.exist():
                    br.utils.subprocess_not_output("service auditd stop")
            if args[SERVICE_AUTOSTART]:
                self.systemd_proxy.enable()
            else:
                self.systemd_proxy.disable()
            return (True, "")
        except Exception as e:
            br.log.error(str(e))
            return (False, "Abnormal service!")


class Rules:
    def __init__(self):
        pass

    # 删除所有审计规则
    def delete_all_rules(self):
        # 读取文件所有行
        with open(AUDIT_RULES_PATH, "r") as file:
            lines = file.readlines()
        # 遍历行
        for line in lines:
            br.utils.subprocess_has_output_ignore_error_handling(
                "auditctl -W {0}".format(line.replace("-w ", ""))
            )
        br.utils.subprocess_not_output("cat /dev/null > {0}".format(AUDIT_RULES_PATH))

    # 添加审计规则
    def add_rule(self, watch_file):
        if len(str(watch_file)) == 0:
            return

        rule = "-w {0} -p rwxa".format(watch_file)
        br.log.info("rule" + rule)
        # 写入文件持久化生效
        br.utils.subprocess_not_output(
            'echo "{0}" >> {1}'.format(rule, AUDIT_RULES_PATH)
        )
        # 执行命令临时生效
        br.utils.subprocess_has_output_ignore_error_handling(
            "auditctl {0}".format(rule)
        )

    def get(self):
        retdata = dict()
        watch_files = list()
        with open(AUDIT_RULES_PATH, "r") as file:
            for line in file.readlines():
                watch_file = line.split()[1]
                watch_files.append(str(watch_file))

        retdata[AUDIT_WATCH_PATH_KEY] = ";".join(watch_files)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        self.delete_all_rules()

        watch_files = args[AUDIT_WATCH_PATH_KEY].split(";")
        for watch_file in watch_files:
            self.add_rule(watch_file)

        return (True, "")

    def backup(self):
        return self.get()

    def rollback(self, args_json):
        return self.set(args_json)