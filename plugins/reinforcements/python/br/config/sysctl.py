# -*- coding: utf-8 -*-

import json
import br.configuration
import br.utils
import br.log
import os
import base64


# 源码https://gitlab.com/procps-ng/procps/-/blob/master/src/sysctl.c
# 源码中先加载 */sysctl.d 下的配置，最后加载 /etc/sysctl.conf
# 与手册不一致，手册中写的 */sysctl.d 会覆盖/etc/sysctl.conf，实际不会
# 所以不需要自定义配置文件，直接修改系统配置即可
SAK_KEY_SWITCH_CONF_SYS_FILE = "/etc/sysctl.conf"
SAK_KEY_SWITCH_CONF_KEY_SYSRQ = "kernel.sysrq"
DMESG_SWITCH_CONF_KEY_SYSRQ = "kernel.dmesg_restrict"

# 开关机快捷键 systemd
SYSTEMD_REBOOT_KEY_STATUS = "systemctl   status  ctrl-alt-del.target"
SYSTEMD_REBOOT_KEY_DISABLE = "systemctl   mask   ctrl-alt-del.target"
SYSTEMD_REBOOT_KEY_ENABLE = "systemctl   unmask   ctrl-alt-del.target"

# 开关机快捷键 dconf
# 新版本的系统配置的是dconf快捷键 （centos7(gnome)、centos8(gnome)、kylinsec(mate)）
SCHEMAS_REBOOT_KEY_CONF_FILE = (
    "/usr/share/glib-2.0/schemas/98-br-config.gschema.override"
)
SCHEMAS_RELOAD = "glib-compile-schemas /usr/share/glib-2.0/schemas"
SCHEMAS_REBOOT_KEY_DISABLE = (
    "echo -e \"[org.mate.SettingsDaemon.plugins.media-keys]\npower=''\n[org.gnome.settings-daemon.plugins.media-keys]\nlogout=''\" > "
    + SCHEMAS_REBOOT_KEY_CONF_FILE
)
SCHEMAS_REBOOT_KEY_ENABLE = "rm -f " + SCHEMAS_REBOOT_KEY_CONF_FILE

# 开关机快捷键 gconf
# centos6桌面配置的是gnome（gconf）快捷键，使用gconftool-2进行配置，注销生效
# 设置时，需要设置现有用户的快捷键，同时设置后续新用户的默认快捷键
# 另外 红帽官方文档介绍的 /etc/init/control-alt-delete.conf 和 /etc/inittab，也要设置，配置的是命令行的快捷键
# 6.10 支持 /etc/init/control-alt-delete.override 覆蓋配置，但 6.0 不支持，需要直接更改 /etc/init/control-alt-delete.conf
# 详见：https://docs.redhat.com/en/documentation/red_hat_enterprise_linux/6/html/deployment_guide/disabling-rebooting-using-ctrl-alt-del

# 判断后台进程是否启动
CHECK_GCONFD_PROC = "pgrep gconfd-2"
# gconf 获取命令：参数为配置文件路径
GCONF_GET_REBOOT_KEYBINDING = "gconftool-2 --direct --config-source xml:readwrite:{}/.gconf --get /apps/gnome_settings_daemon/keybindings/power"
DEFAULT_REBOOT_KEYBINDING = "<Control><Alt>Delete"
# 这里不能用sudo -u 或者 su -l 去改写配置，6.0 sudo命令需要tty，自启进程不支持，6.10不支持 su -l
# gconf 设置命令：第一参数为配置文件路径，第二参数为配置值
GCONF_SET_REBOOT_KEYBINDING = 'gconftool-2 --direct --config-source xml:readwrite:{}/.gconf --set /apps/gnome_settings_daemon/keybindings/power --type string "{}"'

ETC_INIT_REBOOT_CONF = "/etc/init/control-alt-delete.conf"
ETC_INIT_REBOOT_BAK_CONF = (
    "/etc/init/control-alt-delete.conf.bak"  # 这个备份文件用于使能重启快捷键
)
ETC_INIT_REBOOT_CONF_BAK = "mv " + ETC_INIT_REBOOT_CONF + " " + ETC_INIT_REBOOT_BAK_CONF
DISABLE_ETC_INIT_REBOOT_CONF = 'echo "exec true" > ' + ETC_INIT_REBOOT_CONF
ENABLE_ETC_INIT_REBOOT_CONF = (
    "mv " + ETC_INIT_REBOOT_BAK_CONF + " " + ETC_INIT_REBOOT_CONF
)
CHECK_ETC_INIT_REBOOT_CONF = (
    'grep -P "^\s*exec /sbin/(shutdown|reboot)" ' + ETC_INIT_REBOOT_CONF
)

# 新用户的重启快捷键更改
DEFAULT_GCONF_SET_REBOOT_KEYBINDING = 'gconftool-2 --direct --config-source xml:readwrite:/etc/gconf/gconf.xml.defaults --type string --set /apps/gnome_settings_daemon/keybindings/power "{}"'
DEFAULT_GCONF_GET_REBOOT_KEYBINDING = "gconftool-2 --direct --config-source xml:readwrite:/etc/gconf/gconf.xml.defaults --get /apps/gnome_settings_daemon/keybindings/power"
DEFAULT_GCONF = "gconf.xml.defaults"


# todo: 添加加固项是否兼容的接口。
class Sysctl(object):
    def __init__(self, key):
        self.conf = br.configuration.KV(SAK_KEY_SWITCH_CONF_SYS_FILE, "\\s*=\\s*", "=")
        self.key = key

    def available(self):
        isSuccess, stdout, stderr = br.utils.execute_command(
            "sysctl -n {}".format(self.key)
        )
        return isSuccess

    def get_value(self):
        br.utils.subprocess_not_output("sysctl --system", ignore_exception=True)
        # 低版本 centos-6 sysctl 命令没有 --system， 所以使用 sysctl -p /etc/sysctl.conf 使能修改。
        br.utils.subprocess_not_output(
            "sysctl -p {0}".format(SAK_KEY_SWITCH_CONF_SYS_FILE), ignore_exception=True
        )
        cmd = "sysctl -n {}".format(self.key)
        return int(br.utils.subprocess_has_output(cmd))

    def get(self):
        retdata = dict()
        retdata[self.key] = self.get_value()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        value = self.get_value()
        self.conf.set_value(self.key, int(args[self.key]))
        check_value = self.get_value()
        if check_value != value:
            return (False, "Not in effect, {} is set elsewhere".format(self.key))

        return (True, "")

    def backup(self):
        retdata = dict()
        value = self.get_value()
        if value:  # 原样备份
            retdata[self.key] = value
        return (True, json.dumps(retdata))

    def rollback(self, args_json):
        args = json.loads(args_json)
        if self.key in args:  # 原样还原
            self.conf.set_value(self.key, args[self.key])
        else:
            self.conf.del_record(self.key)
        return self.set(args_json)


class SAKKey(Sysctl):
    def __init__(self):
        super(SAKKey, self).__init__(SAK_KEY_SWITCH_CONF_KEY_SYSRQ)


class Dmesg(Sysctl):
    def __init__(self):
        super(Dmesg, self).__init__(DMESG_SWITCH_CONF_KEY_SYSRQ)


class KeyRebootSwitch:
    def reload_schemas(self):
        br.utils.subprocess_not_output(SCHEMAS_RELOAD)

    # 判断文件是否存在
    def systemd_reboot_key_service_exists(self):
        command = "ls /usr/lib/systemd/system/ |grep -wx ctrl-alt-del.target"
        cmd = "{0}".format(command)
        output = br.utils.subprocess_has_output(cmd)
        return len(output) != 0

    def systemd_reboot_key_service_status(self):
        command = "{0} | grep masked".format(SYSTEMD_REBOOT_KEY_STATUS)
        output = br.utils.subprocess_has_output(command)
        return len(output) == 0

    # 判断.bak是否存在
    def status_bak(self):
        command = " ls /usr/lib/systemd/system/ |grep ctrl-alt-del.target.bak"
        cmd = "{0}".format(command)
        output = br.utils.subprocess_has_output(cmd)
        return len(output) != 0

    def open(self):
        br.utils.subprocess_not_output(SYSTEMD_REBOOT_KEY_ENABLE)
        br.utils.subprocess_not_output(SCHEMAS_REBOOT_KEY_ENABLE)
        self.reload_schemas()

    def close(self):
        br.utils.subprocess_not_output(SYSTEMD_REBOOT_KEY_DISABLE)
        br.utils.subprocess_not_output(SCHEMAS_REBOOT_KEY_DISABLE)
        self.reload_schemas()

    def get_on_centos_6(self):
        if len(br.utils.subprocess_has_output(CHECK_GCONFD_PROC)):
            # 现有用户
            for user_home in os.listdir("/home"):
                user_path = os.path.join("/home", user_home)
                if os.path.isdir(user_path):
                    command = GCONF_GET_REBOOT_KEYBINDING.format(user_path)
                    if len(br.utils.subprocess_has_output(command)):
                        return True
            # root
            command = GCONF_GET_REBOOT_KEYBINDING.format("/root")
            if len(br.utils.subprocess_has_output(command)):
                return True
            # 默认配置
            if len(br.utils.subprocess_has_output(DEFAULT_GCONF_GET_REBOOT_KEYBINDING)):
                return True

        else:
            # 不存在gconf服务时，触发警告
            br.log.warning(
                "gconfd-2 do not running, We will not check gconf reboot keybinding"
            )

        # 命令行
        # 默认配置存在重启功能
        if len(br.utils.subprocess_has_output(CHECK_ETC_INIT_REBOOT_CONF)):
            return True

        return False

    def set_on_centos_6(self, is_enable):
        keybinding = DEFAULT_REBOOT_KEYBINDING
        if not is_enable:
            keybinding = ""
        if len(br.utils.subprocess_has_output(CHECK_GCONFD_PROC)):
            # 现有用户设置
            for user_home in os.listdir("/home"):
                user_path = os.path.join("/home", user_home)
                if os.path.isdir(user_path):
                    command = GCONF_SET_REBOOT_KEYBINDING.format(user_path, keybinding)
                    br.utils.subprocess_not_output(command)

            # root设置
            command = GCONF_SET_REBOOT_KEYBINDING.format("/root", keybinding)
            br.utils.subprocess_not_output(command)

            # 新用户和没有自定义该设置的用户
            command = DEFAULT_GCONF_SET_REBOOT_KEYBINDING.format(keybinding)
            br.utils.subprocess_not_output(command)

        else:
            # 不存在gconf服务时，触发警告
            br.log.warning(
                "gconfd-2 do not running, We will not check gconf reboot keybinding"
            )

        # 命令行
        if is_enable:
            br.utils.subprocess_not_output(ENABLE_ETC_INIT_REBOOT_CONF)
        else:
            if not os.path.exists(ETC_INIT_REBOOT_CONF_BAK):  # 不存在备份，先备份
                br.utils.subprocess_not_output(ETC_INIT_REBOOT_CONF_BAK)
            br.utils.subprocess_not_output(DISABLE_ETC_INIT_REBOOT_CONF)

    def get(self):
        retdata = dict()
        retdata["enabled"] = False
        # 判断centos版本是否是6.x
        if br.utils.is_cent_os_6():
            retdata["enabled"] = self.get_on_centos_6()
        else:
            if (
                self.systemd_reboot_key_service_exists()
                and self.systemd_reboot_key_service_status()
            ):
                retdata["enabled"] = True
                # 如果服务使能了，直接返回
                return (True, json.dumps(retdata))

            # 处理glib schemas
            if os.path.exists(SCHEMAS_REBOOT_KEY_CONF_FILE):
                retdata["enabled"] = False

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        # 判断centos版本是否是6.x
        if br.utils.is_cent_os_6():
            self.set_on_centos_6(args["enabled"])
        else:
            # 针对3.3-6的处理规则，文件不存在，开关为打开是，将.bak改为ctrl-alt-del.target
            if args["enabled"] and self.status_bak():
                command = "mv /usr/lib/systemd/system/ctrl-alt-del.target.bak /usr/lib/systemd/system/ctrl-alt-del.target"
                br.utils.subprocess_not_output(command)

            if args["enabled"] and not self.systemd_reboot_key_service_exists():
                return (False, "No related services found")

            # 其他系统处理glib schemas
            if args["enabled"]:
                self.open()
            else:
                self.close()

        return (True, "")

    def backup(self):
        if br.utils.is_cent_os_6():
            retdata = dict()
            if len(br.utils.subprocess_has_output(CHECK_GCONFD_PROC)):
                # 现有用户
                for user_home in os.listdir("/home"):
                    user_path = os.path.join("/home", user_home)
                    if os.path.isdir(user_path):
                        command = GCONF_GET_REBOOT_KEYBINDING.format(user_path)
                        value = br.utils.subprocess_has_output(command)
                        retdata[user_path] = value
                # root
                command = GCONF_GET_REBOOT_KEYBINDING.format("/root")
                value = br.utils.subprocess_has_output(command)
                retdata["/root"] = value

                # 默认配置
                value = br.utils.subprocess_has_output(
                    DEFAULT_GCONF_GET_REBOOT_KEYBINDING
                )
                retdata[DEFAULT_GCONF] = value

            else:
                # 不存在gconf服务时，触发警告
                br.log.warning(
                    "gconfd-2 do not running, We will not check gconf reboot keybinding"
                )

            # ETC_INIT_REBOOT_CONF
            if os.path.exists(ETC_INIT_REBOOT_CONF):
                with open(ETC_INIT_REBOOT_CONF, "r") as file:
                    raw_data = file.read()
                    raw_data_bytes = raw_data.encode("utf-8")
                    encoded_data = base64.b64encode(raw_data_bytes)
                    encoded_str = encoded_data.decode("utf-8")
                    retdata[ETC_INIT_REBOOT_CONF] = encoded_str

            return (True, json.dumps(retdata))

        else:
            return self.get()

    def rollback(self, args_json):
        if br.utils.is_cent_os_6():
            args = json.loads(args_json)
            # 处理 ETC_INIT_REBOOT_CONF 文件
            if ETC_INIT_REBOOT_CONF in args:
                encoded_bytes = args[ETC_INIT_REBOOT_CONF].encode("utf-8")
                decoded_bytes = base64.b64decode(encoded_bytes)
                decoded_string = decoded_bytes.decode("utf-8")

                with open(ETC_INIT_REBOOT_CONF, "w") as file:
                    file.write(decoded_string)

            args.pop(ETC_INIT_REBOOT_CONF, None)

            if len(br.utils.subprocess_has_output(CHECK_GCONFD_PROC)):
                # 处理默认配置
                if DEFAULT_GCONF in args:
                    value = args[DEFAULT_GCONF]
                    command = DEFAULT_GCONF_SET_REBOOT_KEYBINDING.format(value)
                    br.utils.subprocess_not_output(command)

                args.pop(DEFAULT_GCONF, None)

                # 处理各个用户
                for key in args:
                    value = args[key]
                    br.utils.subprocess_not_output(
                        GCONF_SET_REBOOT_KEYBINDING.format(key, value)
                    )
            else:
                # 不存在gconf服务时，触发警告
                br.log.warning(
                    "gconfd-2 do not running, We will not check gconf reboot keybinding"
                )

            return (True, "")
        else:
            return self.set(args_json)
