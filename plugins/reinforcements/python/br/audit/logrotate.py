# -*- coding: utf-8 -*-

import json
import br.configuration
import re
import br.log


LOGROTATE_CONF_PATH = "/etc/logrotate.conf"
ROTATE_ARG_ROTATE = "rotate"
ROTATE_ARG_PERIOD = "period"
VALID_PERIODS = ["daily", "weekly", "monthly", "yearly"]
GLOBAL_PERIOD_REGEX = r"^(daily|weekly|monthly|yearly)$"
GLOBAL_ROTATE_REGEX = r"^rotate\s+(\d+)$"


def get_global_period():
    global_period = None
    with open(LOGROTATE_CONF_PATH, "r") as file:
        lines = file.readlines()

    in_block = False  # 标记是否在文件块中

    for line in lines:
        stripped_line = line.strip()

        # 检测块的开始和结束
        if "{" in stripped_line:
            in_block = True
        elif "}" in stripped_line:
            in_block = False
            continue

        # 仅处理不在块中的日志周期
        if not in_block and re.match(GLOBAL_PERIOD_REGEX, stripped_line):
            global_period = stripped_line  # 匹配的全局日志周期

    return global_period  # 未找到全局日志轮转周期


def set_global_period(new_period):
    br.log.info("set global period {}".format(new_period))

    with open(LOGROTATE_CONF_PATH, "r") as file:
        lines = file.readlines()

    updated_lines = []
    in_block = False  # 标记是否在文件块中
    updated = False  # 标记是否已更新全局日志周期

    for line in lines:
        stripped_line = line.strip()

        # 检测块的开始和结束
        if "{" in stripped_line:
            in_block = True
        elif "}" in stripped_line:
            in_block = False

        # 替换全局日志周期，仅在不在块中时进行替换
        if not in_block and re.match(GLOBAL_PERIOD_REGEX, stripped_line):
            updated_lines.append(new_period + "\n")
            updated = True
        else:
            updated_lines.append(line)

    # 如果未找到全局日志周期，则在文件开头插入新的定义
    if not updated:
        updated_lines.insert(0, new_period + "\n")

    # 直接修改文件内容，将更新后的内容写回
    with open(LOGROTATE_CONF_PATH, "w") as file:
        file.write("".join(updated_lines))


def get_global_rotate():
    global_rotate = None
    with open(LOGROTATE_CONF_PATH, "r") as file:
        lines = file.readlines()

    in_block = False  # 标记是否在文件块中

    for line in lines:
        stripped_line = line.strip()

        # 检测块的开始和结束
        if "{" in stripped_line:
            in_block = True
        elif "}" in stripped_line:
            in_block = False
            continue

        # 仅处理不在块中的 rotate 设置
        if not in_block and re.match(GLOBAL_ROTATE_REGEX, stripped_line):
            match = re.match(GLOBAL_ROTATE_REGEX, stripped_line)
            global_rotate = match.group(1)  # rotate 的值

    return global_rotate


def set_global_rotate(new_rotate):
    br.log.info("set global rotate {}".format(new_rotate))
    if not new_rotate.isdigit() or int(new_rotate) <= 0:
        raise ValueError(
            "Invalid rotate value: {}. It must be a positive integer.".format(
                new_rotate
            )
        )

    with open(LOGROTATE_CONF_PATH, "r") as file:
        lines = file.readlines()

    updated_lines = []
    in_block = False  # 标记是否在文件块中
    updated = False  # 标记是否已更新全局 rotate 设置

    for line in lines:
        stripped_line = line.strip()

        # 检测块的开始和结束
        if "{" in stripped_line:
            in_block = True
        elif "}" in stripped_line:
            in_block = False

        # 替换全局 rotate 设置，仅在不在块中时进行替换
        if not in_block and re.match(GLOBAL_ROTATE_REGEX, stripped_line):
            updated_lines.append("rotate {}\n".format(new_rotate))
            updated = True
        else:
            updated_lines.append(line)

    # 如果未找到全局 rotate 设置，则在文件开头插入新的定义
    if not updated:
        updated_lines.insert(0, "rotate {}\n".format(new_rotate))

    # 直接修改文件内容，将更新后的内容写回
    with open(LOGROTATE_CONF_PATH, "w") as file:
        file.write("".join(updated_lines))


def period_to_day(period):
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


# 日志文件保存周期
class Rotate:
    def get(self):
        retdata = dict()
        rotate_value = get_global_rotate()
        rotation_period = get_global_period()
        if rotate_value:  # 只返回天数
            rotate_value = str(int(rotate_value) * period_to_day(rotation_period))
        retdata[ROTATE_ARG_ROTATE] = "" if not rotate_value else int(rotate_value)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if ROTATE_ARG_ROTATE in args:
            new_rotate = args[ROTATE_ARG_ROTATE]

            set_global_rotate("" if not str(new_rotate) else str(new_rotate))
        if ROTATE_ARG_PERIOD in args:
            new_period = args[ROTATE_ARG_PERIOD]
            try:
                period_index = int(new_period)
            except ValueError:
                error_message = "Invalid rotation period - " + str(new_period)
                return (False, error_message)

            if not (0 <= period_index < len(VALID_PERIODS)):
                error_message = "Invalid rotation period - " + str(new_period)
                return (False, error_message)

            set_global_period(VALID_PERIODS[period_index])

        return (True, "")

    def backup(self):
        retdata = dict()

        global_rotate = get_global_rotate()
        if global_rotate:
            retdata[ROTATE_ARG_ROTATE] = get_global_rotate()

        global_period = get_global_period()
        if global_period and global_period in VALID_PERIODS:
            retdata[ROTATE_ARG_PERIOD] = VALID_PERIODS.index(global_period)

        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        return self.set(args_json)
