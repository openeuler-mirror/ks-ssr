import json
import ssr.configuration
import ssr.utils

UDEV_CONF_FILEPATH = "/etc/udev/rules.d/90-ssr-external.rules"

DISABLE_CDROM_RULE = "KERNEL==\\\"sr0\\\", ENV{UDISKS_IGNORE}=\\\"1\\\""
DISABLE_USB_RULE = "ACTION==\\\"add\\\", SUBSYSTEMS==\\\"usb\\\", DRIVERS==\\\"usb-storage|uas\\\", ATTR{authorized}=\\\"0\\\""
DISABLE_TTYPS_RULE = "KERNEL==\\\"ttyS[0-9]*\\\", SUBSYSTEM==\\\"tty\\\", MODE=\\\"0000\\\""

CDROM_ARG_ENABLED = "enabled"
USB_ARG_ENABLED = "enabled"
TTYS_ARG_ENABLED = "enabled"


class UDev:
    def __init__(self):
        self.conf = ssr.configuration.Table(UDEV_CONF_FILEPATH, ",\\s+")

    def reload(self):
        command =  'udevadm control --reload'
        outpur = ssr.utils.subprocess_has_output(command)

class CDROM(UDev):
    def get(self):
        retdata = dict()
        value = self.conf.get_value("1=KERNEL==\\\"sr0\\\"")
        retdata[CDROM_ARG_ENABLED] = (len(value) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[CDROM_ARG_ENABLED]:
            self.conf.del_record("1=KERNEL==\\\"sr0\\\"")
        else:
            self.conf.set_value("1=KERNEL==\\\"sr0\\\"", DISABLE_CDROM_RULE)

        self.reload()
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
        value = self.conf.get_value("1=KERNEL==\\\"ttyS[0-9]*\\\";3=MODE=\\\"0000\\\"")
        retdata[TTYS_ARG_ENABLED] = (len(value) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if args[TTYS_ARG_ENABLED]:
            self.conf.del_record("1=KERNEL==\\\"ttyS[0-9]*\\\"")
        else:
            self.conf.set_value("1=KERNEL==\\\"ttyS[0-9]*\\\"", DISABLE_TTYPS_RULE)

        self.reload()
        return (True, '')