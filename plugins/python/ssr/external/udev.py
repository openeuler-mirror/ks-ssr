import json
import ssr.configuration
import ssr.utils
import ssr.log

UDEV_CONF_FILEPATH = "/etc/udev/rules.d/90-ssr-external.rules"
DRIVER_BLACKLIST_PATH = "/etc/modprobe.d/ssr-blacklist.conf"
RC_LOCAL_PATH = "/etc/rc.d/rc.local"

#DISABLE_CDROM_RULE = "KERNEL==\\\"sr0\\\", ENV{UDISKS_IGNORE}=\\\"1\\\""
DISABLE_USB_RULE = "ACTION==\\\"add\\\", SUBSYSTEMS==\\\"usb\\\", DRIVERS==\\\"usb-storage|uas\\\", ATTR{authorized}=\\\"0\\\""

TTYPS_CMD_STR = "setserial /dev/ttyS"
TTYPS_STATUS_CMD = "setserial -g /dev/ttyS"
TTYPS_SUM_DEV_CMD = "ls /dev/ttyS* |wc -l"
USB_SUM_DEV_CMD = "ls /dev/ |grep sd"

CDROM_STATUS_CMD = " cat /proc/modules |grep "
CDROM_DRIVE = "cdrom"
SR_MOD_DRIVE = "sr_mod"
FIND_DRIVE_CMD = "find /lib/modules/"
UNINSTALL_DRIVE = "rmmod"
INSTALL_DRIVE = "insmod"
RELOAD_INITRAMFS = "dracut -f"

CDROM_ARG_ENABLED = "enabled"
USB_ARG_ENABLED = "enabled"
TTYS_ARG_ENABLED = "enabled"

class UDev:
    def __init__(self):
        self.conf = ssr.configuration.Table(UDEV_CONF_FILEPATH, ",\\s+")
        self.conf_rc = ssr.configuration.Table(RC_LOCAL_PATH, ",\\s+")
        command =  'chmod +x {0}'.format(RC_LOCAL_PATH)
        outpur = ssr.utils.subprocess_has_output(command)

    def reload(self):
        command =  'udevadm control --reload'
        outpur = ssr.utils.subprocess_has_output(command)

class DRIVERS:
    def __init__(self):
        self.conf = ssr.configuration.Table(DRIVER_BLACKLIST_PATH, ",\\s+")
        self.flag = False

    def find_drive(self,key):
        command = '{0}{1}  -name {2}.ko*'.format(FIND_DRIVE_CMD,str(self.get_kernel_version()),key)
        drivers = ssr.utils.subprocess_has_output(command)
        ssr.log.debug('find_drive = ',drivers)
        return str(drivers)

    def get_kernel_version(self) :
        cmd = 'uname -a'
        output = ssr.utils.subprocess_has_output(cmd)
        kernel_version = str(output).split(' ')
        return kernel_version[2]

    def reload_drive(self,kernel_version):
        ssr.log.debug('kernel_version = ',kernel_version)
        initramfs_name = 'initramfs-' + kernel_version + '.img'
        reload_initramfs = 'cd /boot/ ; {0} {1} ; cd -'.format(RELOAD_INITRAMFS,initramfs_name)
        output = ssr.utils.subprocess_not_output(reload_initramfs)

    def ignore_drive(self,opt_1 = '',opt_2 = '',opt_3 = ''):
        cmd = '{0}  {1}  {2} {3}'.format(RELOAD_INITRAMFS,opt_1,opt_2,opt_3)
        output = ssr.utils.subprocess_not_output(cmd)

class CDROM(DRIVERS):
    def open(self):
        try:
            if not self.status():
                cdrom_drive_path = self.find_drive(CDROM_DRIVE)
                sr_mod_drive_path = self.find_drive(SR_MOD_DRIVE)

                # install the driver
                cmd_cdrom = '{0} {1}'.format(INSTALL_DRIVE,self.find_drive(CDROM_DRIVE))
                cmd_sr_mod = '{0} {1}'.format(INSTALL_DRIVE,self.find_drive(SR_MOD_DRIVE))
                output_cdrom = ssr.utils.subprocess_has_output(cmd_cdrom)
                output_sr_mod = ssr.utils.subprocess_has_output(cmd_sr_mod)

                # change the driver name
                if cdrom_drive_path.find('.bak') > 0:
                    mv_cdrom = 'mv {0} {1}'.format(cdrom_drive_path,cdrom_drive_path.replace('.bak',''))
                    output = ssr.utils.subprocess_not_output(mv_cdrom)
                if sr_mod_drive_path.find('.bak') > 0:
                    mv_sr_mod = 'mv {0} {1}'.format(sr_mod_drive_path,sr_mod_drive_path.replace('.bak',''))
                    output = ssr.utils.subprocess_not_output(mv_sr_mod)

                # reload initramfs
                #self.reload_drive(self.get_kernel_version())

        except Exception as e:
            ssr.log.debug('Exception_open',e)
            return (False,str(e))

    def close(self):
        try:
            if self.status():
                cdrom_drive_path = self.find_drive(CDROM_DRIVE)
                sr_mod_drive_path = self.find_drive(SR_MOD_DRIVE)

                # uninstall the driver
                cmd_cdrom = '{0} {1}'.format(UNINSTALL_DRIVE,CDROM_DRIVE)
                cmd_sr_mod = '{0} {1}'.format(UNINSTALL_DRIVE,SR_MOD_DRIVE)
                output_sr_mod = ssr.utils.subprocess_has_output(cmd_sr_mod)
                output_cdrom = ssr.utils.subprocess_has_output(cmd_cdrom)
                
                # change the driver name and retain the backup
                if cdrom_drive_path.find('.bak') < 0:
                    mv_cdrom = 'mv {0} {1}'.format(cdrom_drive_path,cdrom_drive_path + '.bak')
                    output = ssr.utils.subprocess_not_output(mv_cdrom)
                if not sr_mod_drive_path.find('.bak') < 0:
                    mv_sr_mod = 'mv {0} {1}'.format(sr_mod_drive_path,sr_mod_drive_path + '.bak')
                    output = ssr.utils.subprocess_not_output(mv_sr_mod)

                # reload initramfs
                #self.reload_drive(self.get_kernel_version())

        except Exception as e:
            ssr.log.debug('Exception_close',e)
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
        if not self.flag:
            self.ignore_drive("-o cdrom","-o sr_mod")
            self.flag = True

        if not args[CDROM_ARG_ENABLED]:
            self.conf.set_value("1=blacklist cdrom","blacklist cdrom")
            self.conf.set_value("1=blacklist sr_mod","blacklist sr_mod")
            output = self.close()
            if output:
                return (False,"Device busy , please pop up! \t\t\nYou can use the eject command to eject the device. \t\t")
        else:
            self.conf.del_record("1=blacklist cdrom")
            self.conf.del_record("1=blacklist sr_mod")
            output = self.open()
            if output:
                return (False,"Please contact the admin. \t\t")

        return (True, '')


class USB(UDev):
    def usb_status(self):
        cmd_usb = '{0}'.format(USB_SUM_DEV_CMD)
        output = ssr.utils.subprocess_has_output(cmd_usb)
        return len(output) != 0

    def get(self):
        retdata = dict()
        value = self.conf.get_value("1=ACTION==\\\"add\\\";2=SUBSYSTEMS==\\\"usb\\\"")
        retdata[USB_ARG_ENABLED] = (len(value) == 0)
        return (True, json.dumps(retdata))

    def set(self, args_json):
        args = json.loads(args_json)

        if self.usb_status():
            return (False, 'Device busy , please pop up! \t\t')

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
        
        setserial_status = 'grep  setserial {0} -nR'.format(RC_LOCAL_PATH) 
        output = ssr.utils.subprocess_has_output(setserial_status)
        if  len(output)  == 0:
            retdata[TTYS_ARG_ENABLED]  = True
            return (False, json.dumps(retdata))

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
                ttys_open_cmd = TTYPS_CMD_STR + str(open_index) + "  " + "-a autoconfig"
                ttys_open_command = '{0}'.format(ttys_open_cmd)
                ttys_close_cmd = TTYPS_CMD_STR + str(open_index) + "  " + "uart none"
                self.conf_rc.del_record("1={0}".format(ttys_close_cmd))
                if len(flag_open) != 0:
                    open_output = ssr.utils.subprocess_not_output(ttys_open_command)
                open_index += 1
        else:
            while close_index < output_nums:
                flag_close_cmd = TTYPS_STATUS_CMD + str(close_index)
                command_close_status = '{0} | grep unknown'.format(flag_close_cmd)
                flag_close = ssr.utils.subprocess_has_output(command_close_status)
                ttys_close_cmd = TTYPS_CMD_STR + str(close_index) + "  " + "uart none"
                ttys_close_command = '{0}'.format(ttys_close_cmd)
                self.conf_rc.set_value("1={0}".format(ttys_close_cmd), ttys_close_cmd)
                if len(flag_close) == 0:
                    close_output = ssr.utils.subprocess_not_output(ttys_close_command)
                close_index += 1

        self.reload()
        return (True, '')
