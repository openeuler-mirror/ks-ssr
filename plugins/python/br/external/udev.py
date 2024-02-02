# -*- coding: utf-8 -*-
import json
import br.configuration
import br.utils
import br.log

UDEV_CONF_FILEPATH = "/etc/udev/rules.d/90-br-external.rules"
DRIVER_BLACKLIST_PATH = "/etc/modprobe.d/br-blacklist.conf"
RC_LOCAL_PATH = "/etc/rc.d/rc.local"

DISABLE_USB_RULE = "ACTION==\\\"add\\\", SUBSYSTEMS==\\\"usb\\\", DRIVERS==\\\"usb-storage|uas\\\", ATTR{authorized}=\\\"0\\\""

TTYPS_CMD_STR = "setserial /dev/ttyS"
TTYPS_STATUS_CMD = "setserial -g /dev/ttyS"
TTYPS_SUM_DEV_CMD = "ls /dev/ttyS* |wc -l"
TTYPS_CHECK_CMD_TAIL = "| grep unknown"

USB_SUM_DEV_CMD = "ls /proc/scsi/ |grep usb-storage"

CDROM_STATUS_CMD = " cat /proc/modules |grep "
CDROM_DRIVE = "cdrom"
SR_MOD_DRIVE = "sr_mod"
USB_STORAGE_DRIVE = "usb-storage"
FIND_DRIVE_CMD = "find /lib/modules/"
UNINSTALL_DRIVE = "modprobe -r"
INSTALL_DRIVE = "insmod"
RELOAD_INITRAMFS = "dracut -f"

CDROM_ARG_ENABLED = "enabled"
USB_ARG_ENABLED = "enabled"
TTYS_ARG_ENABLED = "enabled"


class UDev:
    def __init__(self):
        self.conf = br.configuration.Table(UDEV_CONF_FILEPATH, ",\\s+")
        self.conf_rc = br.configuration.Table(RC_LOCAL_PATH, ",\\s+")
        command = 'chmod +x {0}'.format(RC_LOCAL_PATH)
        br.utils.subprocess_not_output(command)

    def reload(self):
        command = 'udevadm control --reload'
        br.utils.subprocess_not_output(command)


class DRIVERS:
    def __init__(self):
        self.conf = br.configuration.Table(DRIVER_BLACKLIST_PATH, ",\\s+")
        self.flag_cdrom = False
        self.flag_usb = False

    def find_drive(self, key):
        command = '{0}{1}  -name {2}.ko*'.format(
            FIND_DRIVE_CMD, str(self.get_kernel_version()), key)
        drivers = br.utils.subprocess_has_output(command)
        br.log.debug('find_drive = ', drivers)
        return str(drivers)

    def get_kernel_version(self):
        cmd = 'uname -r'
        output = br.utils.subprocess_has_output(cmd)
        return output

    def reload_drive(self, kernel_version):
        br.log.debug('kernel_version = ', kernel_version)
        initramfs_name = 'initramfs-' + kernel_version + '.img'
        reload_initramfs = 'cd /boot/ ; {0} {1} ; cd -'.format(
            RELOAD_INITRAMFS, initramfs_name)
        br.utils.subprocess_not_output(reload_initramfs)

    def ignore_drive(self, opt_1='', opt_2='', opt_3=''):
        cmd = '{0}  {1}  {2} {3}'.format(RELOAD_INITRAMFS, opt_1, opt_2, opt_3)
        br.utils.subprocess_not_output(cmd)


class CDROM(DRIVERS):
    def open(self):
        try:
            if not self.cdrom_status():
                cdrom_drive_path = self.find_drive(CDROM_DRIVE)
                sr_mod_drive_path = self.find_drive(SR_MOD_DRIVE)

                # install the driver
                cmd_cdrom = '{0} {1}'.format(
                    INSTALL_DRIVE, self.find_drive(CDROM_DRIVE))
                cmd_sr_mod = '{0} {1}'.format(
                    INSTALL_DRIVE, self.find_drive(SR_MOD_DRIVE))
                br.utils.subprocess_not_output(cmd_cdrom)
                br.utils.subprocess_not_output(cmd_sr_mod)

                # change the driver name
                mv_cmd = "mv"
                if cdrom_drive_path.find('.bak') > 0:
                    mv_cdrom = '{0} {1} {2}'.format(mv_cmd, cdrom_drive_path, cdrom_drive_path.replace('.bak', ''))
                    br.utils.subprocess_not_output(mv_cdrom)
                if sr_mod_drive_path.find('.bak') > 0:
                    mv_sr_mod = '{0} {1} {2}'.format(mv_cmd, sr_mod_drive_path, sr_mod_drive_path.replace('.bak', ''))
                    br.utils.subprocess_not_output(mv_sr_mod)

                # reload initramfs
                # self.reload_drive(self.get_kernel_version())

        except Exception as e:
            br.log.error('Exception_open', e)
            return (False, str(e))

    def close(self):
        try:
            if not self.cdrom_status():
                return
            cdrom_drive_path = self.find_drive(CDROM_DRIVE)
            sr_mod_drive_path = self.find_drive(SR_MOD_DRIVE)

            # uninstall the driver
            cmd_cdrom = '{0} {1}'.format(UNINSTALL_DRIVE, CDROM_DRIVE)
            cmd_sr_mod = '{0} {1}'.format(UNINSTALL_DRIVE, SR_MOD_DRIVE)
            if self.sr_mod_status():
                br.utils.subprocess_has_output(cmd_sr_mod)

            other_mod = str(br.utils.subprocess_has_output(
            "cat /proc/modules |grep cdrom"))
            if len(other_mod) != 0:
                other_names = other_mod.split(" ")[3].split(",")
            else:
                other_names = list()
            for other_name in other_names:
                if other_name == '' or other_name == '-':
                    continue
                br.utils.subprocess_not_output(
                    "{0} {1}".format(UNINSTALL_DRIVE, other_name))

            br.utils.subprocess_not_output(cmd_cdrom)

            # change the driver name and retain the backup
            if cdrom_drive_path.find('.bak') < 0:
                mv_cdrom = 'mv {0} {1}'.format(
                    cdrom_drive_path, cdrom_drive_path + '.bak')
                br.utils.subprocess_not_output(mv_cdrom)
            if not sr_mod_drive_path.find('.bak') < 0:
                mv_sr_mod = 'mv {0} {1}'.format(
                    sr_mod_drive_path, sr_mod_drive_path + '.bak')
                br.utils.subprocess_not_output(mv_sr_mod)

            # reload initramfs
            # self.reload_drive(self.get_kernel_version())

        except Exception as e:
            br.log.debug('Exception_close', e)
            # Close failed recovery state
            if not self.cdrom_status():
                br.utils.subprocess_has_output_ignore_error_handling(
                    '{0} {1}'.format(INSTALL_DRIVE, cdrom_drive_path))
            if not self.sr_mod_status():
                br.utils.subprocess_has_output_ignore_error_handling(
                    '{0} {1}'.format(INSTALL_DRIVE, sr_mod_drive_path))
            return (False, str(e))

    def sr_mod_status(self):
        cmd_sr_mod = '{0} {1}'.format(CDROM_STATUS_CMD, SR_MOD_DRIVE)
        output_sr_mod = br.utils.subprocess_has_output(cmd_sr_mod)
        return len(output_sr_mod) != 0

    def cdrom_status(self):
        cmd_cdrom = '{0} {1}'.format(CDROM_STATUS_CMD, CDROM_DRIVE)
        output_cmd_cdrom = br.utils.subprocess_has_output(cmd_cdrom)
        return len(output_cmd_cdrom) != 0

    def get(self):
        retdata = dict()
        retdata[CDROM_ARG_ENABLED] = self.cdrom_status()
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if not self.flag_cdrom:
            self.ignore_drive("-o cdrom", "-o sr_mod")
            self.flag_cdrom = True

        if not args[CDROM_ARG_ENABLED]:
            self.conf.set_value("1=blacklist cdrom", "blacklist cdrom")
            self.conf.set_value("1=blacklist sr_mod", "blacklist sr_mod")
            output = self.close()
            if output:
                return (False, "Device busy, please pop up!")
        else:
            self.conf.del_record("1=blacklist cdrom")
            self.conf.del_record("1=blacklist sr_mod")
            output = self.open()
            if output:
                return (False, "Please contact the admin.")

        return (True, '')


class USB(DRIVERS):
    def usb_status(self):
        cmd_usb = '{0}'.format(USB_SUM_DEV_CMD)
        output = br.utils.subprocess_has_output(cmd_usb)
        return len(output) != 0

    def open(self):
        try:
            if not self.status():
                usb_storage_drive_path = self.find_drive(USB_STORAGE_DRIVE)

                # install the driver
                cmd_usb_storage = '{0} {1}'.format(
                    INSTALL_DRIVE, self.find_drive(USB_STORAGE_DRIVE))
                br.utils.subprocess_not_output(
                    cmd_usb_storage)

                # change the driver name
                if usb_storage_drive_path.find('.bak') > 0:
                    mv_usb_storage = 'mv {0} {1}'.format(
                        usb_storage_drive_path, usb_storage_drive_path.replace('.bak', ''))
                    br.utils.subprocess_not_output(mv_usb_storage)

                # reload initramfs
                # self.reload_drive(self.get_kernel_version())

        except Exception as e:
            br.log.debug('Exception_open', e)
            return (False, str(e))

    def close(self):
        try:
            if self.status():
                usb_storage_drive_path = self.find_drive(USB_STORAGE_DRIVE)

                # uninstall the driver
                cmd_usb_storage = '{0} {1}'.format(
                    UNINSTALL_DRIVE, USB_STORAGE_DRIVE)
                cmd_uas = '{0} {1}'.format(UNINSTALL_DRIVE, "uas")

                if len(br.utils.subprocess_has_output("lsmod |grep usb |grep uas")) != 0:
                    br.utils.subprocess_not_output(cmd_uas)
                    br.utils.subprocess_not_output(cmd_usb_storage)
                else:
                    br.utils.subprocess_not_output(cmd_usb_storage)

                # change the driver name and retain the backup
                if usb_storage_drive_path.find('.bak') < 0:
                    mv_usb_storage = 'mv {0} {1}'.format(
                        usb_storage_drive_path, usb_storage_drive_path + '.bak')
                    br.utils.subprocess_not_output(mv_usb_storage)

                # reload initramfs
                # self.reload_drive(self.get_kernel_version())

        except Exception as e:
            br.log.debug('Exception_close', e)
            return (False, str(e))

    def status(self):
        cmd_usb_mod = '{0} {1}'.format(CDROM_STATUS_CMD, "usb_storage")
        output_usb_mod = br.utils.subprocess_has_output(cmd_usb_mod)
        return len(output_usb_mod) != 0

    def get(self):
        retdata = dict()
        retdata[USB_ARG_ENABLED] = self.status() or (len(br.utils.subprocess_has_output(
            "cat {0} |grep {1}".format(DRIVER_BLACKLIST_PATH, USB_STORAGE_DRIVE))) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)
        if not self.flag_usb:
            br.utils.subprocess_has_output_ignore_error_handling(
                "modprobe {0}".format(USB_STORAGE_DRIVE))
            self.ignore_drive("-o {0}".format(USB_STORAGE_DRIVE))
            self.flag_usb = True

        if not args[USB_ARG_ENABLED]:
            self.conf.set_value("1=blacklist usb-storage",
                                "blacklist usb-storage")
            output = self.close()
            if output:
                return (False, _("Device busy , please pop up!\t"))
        else:
            self.conf.del_record("1=blacklist usb-storage")
            output = self.open()
            if output:
                return (False, _("Please contact the admin.\t"))

        return (True, '')


class TTYS(UDev):
    def get(self):
        retdata = dict()
        index_get = 0
        list_get = []
        nums_command_get = '{0}'.format(TTYPS_SUM_DEV_CMD)
        output_get = br.utils.subprocess_has_output(nums_command_get)
        output_nums_get = int(output_get)
        while int(index_get) < output_nums_get:
            flag_cmd = TTYPS_STATUS_CMD + str(index_get)
            command_status = '{0} {1}'.format(flag_cmd, TTYPS_CHECK_CMD_TAIL)
            flag = br.utils.subprocess_has_output(command_status)
            if len(flag) != 0:
                list_get.append("disabled")
            else:
                list_get.append("enabled")
            index_get += 1

        setserial_status = 'grep  setserial {0} -nR'.format(RC_LOCAL_PATH)
        output = br.utils.subprocess_has_output(setserial_status)
        if len(output) == 0:
            retdata[TTYS_ARG_ENABLED] = True
            return (True, json.dumps(retdata))

        retdata[TTYS_ARG_ENABLED] = "enabled" in list_get

        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        open_index = 0
        close_index = 0
        nums_command = '{0}'.format(TTYPS_SUM_DEV_CMD)
        output = br.utils.subprocess_has_output(nums_command)
        output_nums = int(output)

        if args[TTYS_ARG_ENABLED]:
            while open_index < output_nums:
                flag_open_cmd = TTYPS_STATUS_CMD + str(open_index)
                command_open_status = '{0} {1}'.format(flag_open_cmd, TTYPS_CHECK_CMD_TAIL)
                flag_open = br.utils.subprocess_has_output(
                    command_open_status)
                ttys_open_cmd = TTYPS_CMD_STR + \
                    str(open_index) + "  " + "-a autoconfig"
                ttys_open_command = '{0}'.format(ttys_open_cmd)
                ttys_close_cmd = TTYPS_CMD_STR + \
                    str(open_index) + "  " + "uart none"
                self.conf_rc.del_record("1={0}".format(ttys_close_cmd))
                if len(flag_open) != 0:
                    br.utils.subprocess_not_output(ttys_open_command)
                open_index += 1
        else:
            while close_index < output_nums:
                flag_close_cmd = TTYPS_STATUS_CMD + str(close_index)
                command_close_status = '{0} {1}'.format(flag_close_cmd, TTYPS_CHECK_CMD_TAIL)
                flag_close = br.utils.subprocess_has_output(
                    command_close_status)
                ttys_close_cmd = TTYPS_CMD_STR + \
                    str(close_index) + "  " + "uart none"
                ttys_close_command = '{0}'.format(ttys_close_cmd)
                self.conf_rc.set_value("1={0}".format(
                    ttys_close_cmd), ttys_close_cmd)
                if len(flag_close) == 0:
                    br.utils.subprocess_not_output(ttys_close_command)
                close_index += 1

        self.reload()
        return (True, '')
