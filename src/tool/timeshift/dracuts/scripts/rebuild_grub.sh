export WORK_DIR=/opt/ks-ssr/ks-ssr

function fn_rebuild_grub_core()
{
    name=Linux
    if [ -f "/etc/system-release" ];then
        strsysr=$(cat /etc/system-release)
        if [[ "$strsysr" =~ "Red Hat" ]]
        then
            vendor="redhat"
            name="Red Hat Linux"
        elif [[ "$strsysr" =~ "CentOS release 6"  ]]
        then
            vendor="redhat"
            name="CentOS Linux"
        elif [[ "$strsysr" =~ "CentOS" ]]
        then
            vendor="centos"
            name="CentOS Linux"
        fi
    fi

    if [ -f "/usr/sbin/grub2-probe" ];then
        boot_partition=$(grub2-probe --target=device /boot)
        blk_dev=$(lsblk -spnlo name $boot_partition | head -n2 |tail -n1)
    else
        grubbootpart=`cat /etc/fstab  | grep \/boot | head -1 |awk '{print $1}'`
        if [[ "$grubbootpart" =~ "UUID" ]];then
            boot_partition=`blkid |sed "s/\"//g"|  grep ${grubbootpart} | awk -F : '{ print $1 }'`
        else
            boot_partition=$grubbootpart
        fi
        blk_dev=$(echo $boot_partition | tr -d '0-9')
    fi
	if [ -z "$blk_dev" ];then
        log_error -s "can't find boot device in your system"
        return $FAILED
    fi

    if [ -d "/sys/firmware/efi" ];then
        efifile=`find /boot/ -name "grub*.efi" | grep "$vendor" | awk -F \/efi '{print $2}'|sed 's/\//\\\/g'`
        if [ ! -z "$efifile" ];then
            efibootmgr -c -w -L "$name" -d $blk_dev -p 1 -l "$efifile" > /dev/null 2>&1
        else
            if [ `arch` == 'aarch64' ];then
                efibootmgr -c -w -L "$name" -d $blk_dev -p 1 -l '\EFI\'$vendor'\shimaa64.efi' > /dev/null 2>&1
            else
                efibootmgr -c -w -L "$name" -d $blk_dev -p 1 -l '\EFI\'$vendor'\shimx64.efi' > /dev/null 2>&1
            fi
        fi
    else
        if [ -f "/usr/sbin/grub2-install" ];then
            grub2-install $blk_dev > /dev/null 2>&1
        elif [ -f "/sbin/grub-install" ];then
            /sbin/grub-install $blk_dev > /dev/null 2>&1 
        fi
    fi
    if [ $? -ne 0 ]; then
        log_error -s "fail to rebuild grub after upgrade"
        return $FAILED
    fi
    sync

    return $SUCCESS
}

source $WORK_DIR/global_env_variable.sh
source $WORK_DIR/log.sh
fn_rebuild_grub_core "$@"
