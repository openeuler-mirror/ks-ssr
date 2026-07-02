import json
import ssr.configuration
import ssr.utils
import ssr.log

UDEV_CONF_FILEPATH = "/etc/udev/rules.d/90-ssr-external.rules"

#DISABLE_CDROM_RULE = "KERNEL==\\\"sr0\\\", ENV{UDISKS_IGNORE}=\\\"1\\\""
DISABLE_USB_RULE = "ACTION==\\\"add\\\", SUBSYSTEMS==\\\"usb\\\", DRIVERS==\\\"usb-storage|uas\\\", ATTR{authorized}=\\\"0\\\""

TTYPS_CMD_STR = "setserial /dev/ttyS"
TTYPS_STATUS_CMD = "setserial -g /dev/ttyS"
TTYPS_SUM_DEV_CMD = "ls /dev/ttyS* |wc -l"

CDROM_STATUS_CMD = " cat /proc/modules |grep "
CDROM_DRIVE = "cdrom"
SR_MOD_DRIVE = "sr_mod"
FIND_DRIVE_CMD = "find /lib/modules/ -name"
UNINSTALL_DRIVE = "rmmod"
INSTALL_DRIVE = "insmod"

CDROM_ARG_ENABLED = "enabled"
USB_ARG_ENABLED = "enabled"
TTYS_ARG_ENABLED = "enabled"


class UDev:
    def __init__(self):
        self.conf = ssr.configuration.Table(UDEV_CONF_FILEPATH, ",\\s+")

    def reload(self):
        command =  'udevadm control --reload'
        outpur = ssr.utils.subprocess_has_output(command)

class CDROM():
    def find_drive(self,key):
        command = '{0} {1}.ko*'.format(FIND_DRIVE_CMD,key)
        output = ssr.utils.subprocess_has_output(command)
        return str(output)

    def open(self):
        try:
            if not self.status():
                cmd_cdrom = '{0} {1}'.format(INSTALL_DRIVE,self.find_drive(CDROM_DRIVE))
                cmd_sr_mod = '{0} {1}'.format(INSTALL_DRIVE,self.find_drive(SR_MOD_DRIVE))
                output_cdrom = ssr.utils.subprocess_has_output(cmd_cdrom)
                output_sr_mod = ssr.utils.subprocess_has_output(cmd_sr_mod)
        except Exception as e:
            ssr.log.debug(e)
            return (False,str(e))

    def close(self):
        try:
            if self.status():
                cmd_cdrom = '{0} {1}'.format(UNINSTALL_DRIVE,CDROM_DRIVE)
                cmd_sr_mod = '{0} {1}'.format(UNINSTALL_DRIVE,SR_MOD_DRIVE)
                output_sr_mod = ssr.utils.subprocess_has_output(cmd_sr_mod)
                output_cdrom = ssr.utils.subprocess_has_output(cmd_cdrom)
        except Exception as e:
            ssr.log.debug(e)
            return (False,str(e))

    def status(self):
        cmd_sr_mod = '{0} {1}'.format(CDROM_STATUS_CMD,SR_MOD_DRIVE)
        output_sr_mod = ssr.utils.subprocess_has_output(cmd_sr_mod)
        return len(output_sr_mod) != 0

    def get(self):
        retdata = dict()
        retdata[CDROM_ARG_ENABLED] = self.status()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if not args[CDROM_ARG_ENABLED]:
            output = self.close()
            if output:
                return (False,"Device busy , please pop up! \t")
        else:
            output = self.open()
            if output:
                return (False,"Please contact the admin.  \t")

        return (True, '')


class USB(UDev):
    def get(self):
        retdata = dict()
        value = self.conf.get_value("1=ACTION==\\\"add\\\";2=SUBSYSTEMS==\\\"usb\\\"")
        retdata[USB_ARG_ENABLED] = (len(value) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[USB_ARG_ENABLED]:
            self.conf.del_record("1=ACTION==\\\"add\\\";2=SUBSYSTEMS==\\\"usb\\\"")
        else:
            self.conf.set_value("1=ACTION==\\\"add\\\";2=SUBSYSTEMS==\\\"usb\\\"", DISABLE_USB_RULE)

        self.reload()
        return (True, '')


class TTYS(UDev):

    def get(self):
        retdata = dict()
        index_get = 0
        list_get = []
        nums_command_get = '{0}'.format(TTYPS_SUM_DEV_CMD)
        output_get = ssr.utils.subprocess_has_output(nums_command_get)
        output_nums_get = int(output_get)
        while int(index_get) < output_nums_get:
            flag_cmd = TTYPS_STATUS_CMD + str(index_get)
            command_status = '{0} | grep unknown'.format(flag_cmd)
            flag = ssr.utils.subprocess_has_output(command_status)
            if len(flag) != 0:
                list_get.append("disabled")
            else:
                list_get.append("enabled")
            index_get += 1

        retdata[TTYS_ARG_ENABLED] = "enabled" in list_get

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        open_index = 0
        close_index = 0
        nums_command = '{0}'.format(TTYPS_SUM_DEV_CMD)
        output = ssr.utils.subprocess_has_output(nums_command)
        output_nums = int(output)

        if args[TTYS_ARG_ENABLED]:
            while open_index < output_nums:
                flag_open_cmd = TTYPS_STATUS_CMD + str(open_index)
                command_open_status = '{0} | grep unknown'.format(flag_open_cmd)
                flag_open = ssr.utils.subprocess_has_output(command_open_status)
                if len(flag_open) != 0:
                    ttys_open_cmd = TTYPS_CMD_STR + str(open_index) + "  " + "-a autoconfig"
                    ttys_open_command = '{0}'.format(ttys_open_cmd)
                    open_output = ssr.utils.subprocess_not_output(ttys_open_command)
                open_index += 1
        else:
            while close_index < output_nums:
                flag_close_cmd = TTYPS_STATUS_CMD + str(close_index)
                command_close_status = '{0} | grep unknown'.format(flag_close_cmd)
                flag_close = ssr.utils.subprocess_has_output(command_close_status)
                if len(flag_close) == 0:
                    ttys_close_cmd = TTYPS_CMD_STR + str(close_index) + "  " + "uart none"
                    ttys_close_command = '{0}'.format(ttys_close_cmd)
                    close_output = ssr.utils.subprocess_not_output(ttys_close_command)
                close_index += 1

        self.reload()
        return (True, '')
