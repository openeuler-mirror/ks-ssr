# -*- coding: utf-8 -*-

import json
import br.configuration

LOGROTATE_CONF_PATH = "/etc/logrotate.conf"
ROTATE_ARG_ROTATE = "rotate"
ROTATE_ARG_PERIOD = "period"
VALID_PERIODS = ["daily", "weekly", "monthly", "yearly"]


# 日志文件保存周期
class Rotate:

    def __init__(self):
        self.conf = br.configuration.KV(LOGROTATE_CONF_PATH)

    def period_to_day(self, period):
        rotation_day = 1
        if period == "weekly":
            rotation_day = 7
        elif period == "monthly":
            rotation_day = 30
        elif period == "yearly":
            rotation_day = 365
        else:
            rotation_day = 1
        return rotation_day

    def get_rotation_period(self):
        last_period = None
        with open(LOGROTATE_CONF_PATH, "r") as file:
            for line in file:
                line = line.strip()
                if not line or line.startswith("#"):  # 跳过空行和注释
                    continue

                # 检查周期设置
                if line in VALID_PERIODS:
                    last_period = line  # 更新最后一个周期
        return last_period

    def set_rotation_period(self, new_period):
        period_index = int(new_period)
        if period_index >= len(VALID_PERIODS) or period_index < 0:
            return "Invalid rotation period - " + str(new_period)

        lines = []
        insert_position = None  # 记录插入位置
        for i, line in enumerate(open(LOGROTATE_CONF_PATH, "r")):
            stripped_line = line.strip()

            # 检查是否为周期设置或带注释的周期设置
            if stripped_line.split("#")[0].strip() in VALID_PERIODS:
                insert_position = i  # 记录最后一个有效行的位置
                continue  # 删除该行

            lines.append(line)  # 保留其他行

        # 如果找到周期设置，插入新的周期
        if insert_position is not None:
            lines.insert(insert_position, VALID_PERIODS[period_index] + "\n")
        else:
            lines.append(
                "\n" + VALID_PERIODS[period_index] + "\n"
            )  # 如果没有周期设置，直接添加

        with open(LOGROTATE_CONF_PATH, "w") as file:
            file.writelines(lines)

        return ""

    def get(self):
        retdata = dict()
        rotate_value = self.conf.get_value(ROTATE_ARG_ROTATE)
        rotation_period = self.get_rotation_period()
        if rotate_value:
            rotate_value = str(int(rotate_value) * self.period_to_day(rotation_period))
        retdata[ROTATE_ARG_ROTATE] = "" if not rotate_value else int(rotate_value)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if ROTATE_ARG_ROTATE in args:
            self.conf.set_value(
                ROTATE_ARG_ROTATE,
                "" if not str(args[ROTATE_ARG_ROTATE]) else args[ROTATE_ARG_ROTATE],
            )
        if ROTATE_ARG_PERIOD in args:
            error_msg = self.set_rotation_period(args[ROTATE_ARG_PERIOD])
            if error_msg:
                return (False, error_msg)

        return (True, "")

    def backup(self):
        retdata = dict()
        retdata[ROTATE_ARG_ROTATE] = self.conf.get_value(ROTATE_ARG_ROTATE)
        retdata[ROTATE_ARG_PERIOD] = self.get_rotation_period()
        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        return self.set(args_json)
