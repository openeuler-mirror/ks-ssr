#!/bin/bash

if [ -d /usr/lib64/ks-ssr/bin ];then
    export PATH=/usr/lib64/ks-ssr/bin:$PATH
fi
if [ -d /usr/lib/ks-ssr/bin ];then
    export PATH=/usr/lib/ks-ssr/bin:$PATH
fi
export WORK_DIR="/opt/ks-ssr/ks-ssr"
declare -A FILTER_LIST=()

################################## COMMON ####################################

function usage()
{
    cat <<-EOF
Usage:
        execute [options]

Options:
        upgrade            execute upgrade system
        rollback           execute rollback system
        -h, --help               show help information

EOF
}

function get_config_path()
{
    if [ ! -f "$KS_SSR_CONF" ]; then
        log_error -s "ks-ssr config file not exist."
        return $FILE_NOT_FOUND
    fi

    while read line
    do
        [ -z "$line" ] && continue
        echo "$line" | grep "^#" > /dev/null 2>&1
        [ $? -eq 0 ] && continue

        # match key=value
        key=$(echo $line | awk -F '=' '{print $1}' | awk '{sub(/^ */, "");sub(/ *$/, "")}1')
        if [ -z "$key" ]; then
            log_error -s "ks-ssr config key can not be null."
            return $INVALID_CONFIG
        fi

        if [ "x$key" == "x$1" ];then
            value=$(echo $line | awk -F '=' '{print $2}' | awk '{sub(/^ */, "");sub(/ *$/, "")}1')
            if [ -z "$value" ]; then
                log_info -s "$1 value is null"
                return $INVALID_CONFIG
            fi
            export GET_PATH=$value
            return $SUCCESS
        fi
    done < "$KS_SSR_CONF"

    return $FAILED
}

function get_store_path()
{
    get_config_path "store_path"
    if [ $? -eq 0 ];then
        export OSBAK=$GET_PATH
        export BACKUP_OS_VERSION="$SYSROOT$OSBAK/etc/KylinSecOS-latest"
        export BACKUP_OS_VERSION_CONF="$SYSROOT$OSBAK/etc/KylinSecLinux.conf"
        return $SUCCESS
    fi

    return $FAILED
}

function get_exclude_dir()
{
    get_config_path "exclude_dir"
    if [ $? -eq 0 ];then
        export EXCLUDE_DIR=$GET_PATH
        return $SUCCESS
    fi

    return $FAILED
}

function get_dir_name_and_depth()
{
    local dir_name_tmp=$1
    local dir_depth=0
    export DIR_TOP_NAME=""
    export DIR_DEPTH=0

    while [ $dir_depth -lt 20 ]
    do
        dir_depth=$(expr $dir_depth + 1)
        dir_basename_tmp=$(basename $dir_name_tmp)
        dir_name_tmp=$(dirname $dir_name_tmp)
        if [ "x$dir_name_tmp" == "x/" ];then
            DIR_TOP_NAME=$dir_basename_tmp
            DIR_DEPTH=$dir_depth
            break
        fi
    done

    if [ -z "$DIR_TOP_NAME" ];then
        log_info -s "failed to get dir name"
        return $INVALID_CONFIG
    fi
    return $SUCCESS
}

function get_filter_list()
{
    FILTER_LIST=()
    local rollback_path=$1
    local exclude_path_count=0

    if [ -z "$EXCLUDE_DIR" ]; then
        log_info -s "exclude dir is null"
        return $INVALID_CONFIG
    fi

    exclude_dir_list=$(echo $EXCLUDE_DIR | awk -F ',' '{for(i=1;i<=NF;i++){print $i}}' | awk '{gsub(/^\s+|\s+$/, "");print}')
    for exclude_dir_tmp in $exclude_dir_list
    do
        get_dir_name_and_depth $exclude_dir_tmp
        if [ $? -ne 0 ]; then
            return $FILE_NOT_FOUND
        fi
        if [ "x/$DIR_TOP_NAME" == "x$rollback_path" ]; then
            exclude_dir_file=${exclude_dir_tmp#*$DIR_TOP_NAME/}
            exclude_dir_file="/$DIR_TOP_NAME/$exclude_dir_file"
            log_info -s "add $exclude_dir_file to filter list while rollback $rollback_path"
            FILTER_LIST[$exclude_path_count]="--filter=- $exclude_dir_file"
            exclude_path_count=$(expr $exclude_path_count + 1)
        fi
    done

    if [ $exclude_path_count -eq 0 ]; then
        log_info -s "filter list is null, no need exclude"
        return $FILE_NOT_FOUND
    fi

    return $SUCCESS
}

function file_lock()
{
    local lock_file=$1
    exec {lock_fd}>"$lock_file"
    flock -xn "$lock_fd"
}

function handle_signal_trap()
{
    rm -rf "$KS_SSR_LOCK"
    log_error -s "Interrupt signal received, exit."
    exit $TRAP_SIG_EXIT
}

function do_prepare()
{
    local param=$1
    local root_path=$2

    if [ "x$param" == "xrescue" ]; then
        if [ -d "$root_path" ]; then
            WORK_DIR="/usr/lib/dracut/modules.d/99ksssr/"
            source $WORK_DIR/global_env_variable.sh
            source $WORK_DIR/log.sh
            SYSROOT=`readlink -f $root_path`
            OS_VERSION="$SYSROOT/etc/KylinSecOS-latest"
            OS_VERSION_CONF="$SYSROOT/etc/KylinSecLinux.conf"
            KS_SSR_CONF="$SYSROOT/usr/share/ks-ssr/ssr.ini"
            ROLLBACK_SUCCESS_FLAG="$SYSROOT/opt/ks-ssr/rollback_flag"
            log_info -s "rescue system, new root path is $SYSROOT"
        fi
    fi

    trap "handle_signal_trap" INT TERM QUIT
    file_lock "$KS_SSR_LOCK"
    if [ $? -ne 0 ]; then
        log_error -s "There is already an ks-ssr process running."
        exit $PROCESS_ALREAD_EXIST
    fi

    # remount /sysroot to rw
    mount -o "remount,rw" $SYSROOT
    if [ $? -ne 0 ];then
        return $COMMAND_ERROR
    fi
    log_info -s "Remount $SYSROOT read/write sucessed."

    # mount all disk in fstab
    if [ -f "/usr/sbin/vgchange" ] || [ -f "/sbin/vgchange" ];then
        vgchange -a y > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            log_error -s "failed to activate logical volume."
            return $COMMAND_ERROR
        fi
    fi

    if [ "x$param" == "xupgrade" ]; then
        log_info "Mount bind some necessary directories in /sysroot."
        mount_bind
        if [ $? -ne 0 ];then
            log_error -s "Failed to mount bind some directories."
            return $COMMAND_ERROR
        fi
    else
        log_info "Mount all disks from fstab."

        mount | grep " $SYSROOT/dev " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            log_info -s "mount_point $SYSROOT/dev doesn't need bind."
        else
            mount --bind /dev $SYSROOT/dev
            if [ $? -ne 0 ];then
                log_error -s "Failed to mount bind dev."
                return $COMMAND_ERROR
            fi
        fi

        mount | grep " $SYSROOT/sys " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            log_info -s "mount_point $SYSROOT/sys doesn't need bind."
        else
            mount --bind /sys $SYSROOT/sys
            if [ $? -ne 0 ];then
                log_error -s "Failed to mount bind sys."
                return $COMMAND_ERROR
            fi
        fi

        mount | grep " $SYSROOT/proc " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            log_info -s "mount_point $SYSROOT/proc doesn't need bind."
        else
            mount --bind /proc $SYSROOT/proc
            if [ $? -ne 0 ];then
                log_error -s "Failed to mount bind proc."
                return $COMMAND_ERROR
            fi
        fi

        mount | grep " $SYSROOT/sys/firmware/efi/efivars " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            log_info -s "mount_point efivars doesn't need bind."
        else
            if [ -d "/sys/firmware/efi/efivars" ];then
                mount --bind /sys/firmware/efi/efivars $SYSROOT/sys/firmware/efi/efivars
                if [ $? -ne 0 ];then
                    log_error -s "Failed to mount bind efivars"
                    return $COMMAND_ERROR
                fi
            fi
        fi

        if [ "x$param" == "xrollback" ]; then
            /usr/sbin/chroot $SYSROOT /bin/bash -c "mount -a"
            log_info -s "start check_fstab_devs"
            check_fstab_devs $param
            if [ $? -ne 0 ]; then
                log_info -s "check_fstab_devs not 0"
                log_info -s "start mount_fstab_devs"
                mount_fstab_devs $param
                if [ $? -ne 0 ]; then
                    log_error -s "failed to mount disks in fstab."
                    return $MOUNT_ERROR
                fi
                log_info -s "start check_fstab_devs"

                check_fstab_devs $param
                if [ $? -ne 0 ]; then
                    log_error -s "check fstab devs failed."
                    return $MOUNT_ERROR
                fi
                log_info -s "end check_fstab_devs"
            fi
        elif [ "x$param" == "xrescue" ]; then
            mount_fstab_devs $param
            if [ $? -ne 0 ]; then
                log_error -s "failed to mount disks in fstab."
                return $MOUNT_ERROR
            fi

            check_fstab_devs $param
            if [ $? -ne 0 ]; then
                log_error -s "check fstab devs failed."
                return $MOUNT_ERROR
            fi
        fi
    fi

    get_store_path
    if [ $? -ne 0 ]; then
        log_error -s "read store path from ks-ssr conf error"
        return $FAILED
    fi

    get_exclude_dir
    if [ $? -ne 0 ]; then
        log_info -s "exclude dir is null"
    fi

    log_info "set selinux to disable."
    if [ -f /sys/fs/selinux/enforce ];then
        echo 0 > /sys/fs/selinux/enforce
    fi

    return $SUCCESS
}

################################# ROLLBACK ##################################

function is_need_rollback()
{
    local current_os_ver=""
    local store_os_ver=""

    if [ -f "$OS_VERSION" ];then
        current_os_ver=$(cat "$OS_VERSION" | grep -E "kylinsec" |awk -F '=' '{print $2}')
    current_os_ver=$(echo $current_os_ver|sed "s/_Server//g")
    elif [ -f "$OS_VERSION_CONF" ];then
        current_os_ver=$(cat "$OS_VERSION_CONF" | grep -E "kylinsec" |awk -F '=' '{print $2}')
    current_os_ver=$(echo $current_os_ver|sed "s/_Server//g")
    else
        return $FILE_NOT_FOUND
    fi

    if [ -f "$BACKUP_OS_VERSION" ];then
        store_os_ver=$(cat "$BACKUP_OS_VERSION" | grep -E "kylinsec" |awk -F '=' '{print $2}')
        store_os_ver=$(echo $store_os_ver |sed "s/_Server//g")
    elif [ -f "$BACKUP_OS_VERSION_CONF" ];then
        current_os_ver=$(cat "$BACKUP_OS_VERSION_CONF" | grep -E "kylinsec" |awk -F '=' '{print $2}')
        current_os_ver=$(echo $current_os_ver|sed "s/_Server//g")
    else
        return $FILE_NOT_FOUND
    fi

    if [[ $current_os_ver > $store_os_ver ]];then
        return $SUCCESS
    fi
    log_error -s "Current OS version:$current_os_ver is lower than Backup OS version:$store_os_ver"
    return $NO_NEED_ROLLBACK
}

function rollback_key_files()
{
    local ret=$SUCCESS
    local key_files=("/var" "/usr" "/etc" "/boot")

    for key_file in ${key_files[@]}
    do
        line="$SYSROOT$OSBAK$key_file"
        if [ $line == "$SYSROOT$OSBAK/boot" ]; then
            rm -rf $SYSROOT/boot/* > /dev/null 2>&1
            rsync -a -I --delete "$line" "$SYSROOT"
        else
            get_filter_list $key_file
            if [ $? -eq 0 ]; then
                rsync -a -I --delete "${FILTER_LIST[@]}" "$line" "$SYSROOT"
            else
                rsync -a -I --delete "$line" "$SYSROOT"
            fi
        fi
        ret=$?

        if [ $ret -ne 0 ]; then
            log_error -s "Rollback $line failed, errno $ret."
            sync
            return $ROLLBACK_ERROR
        fi
        log_info -s "Rollback $line success"
        sync
    done

    return $SUCCESS
}

function do_rollback()
{
    local param=$1

    if [ -f "$ROLLBACK_SUCCESS_FLAG" ]; then
        rm -f "$ROLLBACK_SUCCESS_FLAG"
        if [ $? -ne 0 ]; then
            log_error -s "rm $ROLLBACK_SUCCESS_FLAG failed."
            return $COMMAND_ERROR
        fi
    fi

    if [ "x$param" == "x--force" ];then
        log_info -s "Your system will rollback."
    else
        is_need_rollback
        if [ $? -ne 0 ];then
            log_error -s "Your system do not need to rollback."
            exit $NO_NEED_ROLLBACK
        fi
    fi

    if [ -z "$OSBAK" ]; then
        log_error -s "store path can not be null."
        return $INVALID_CONFIG
    fi

    if [ ! -d "$SYSROOT$OSBAK" ]; then
        log_error -s "Backup path $SYSROOT$OSBAK not exist, exit."
        return $FILE_NOT_FOUND
    fi

    if [ "`ls -A $SYSROOT$OSBAK`" = "" ]; then
        log_error -s "Backup path $SYSROOT$OSBAK is empty, exit."
        return $FILE_NOT_FOUND
    fi

    log_info -s "Start to rollback the system."
    for line in $SYSROOT$OSBAK/*
    do
    if [ $line == "$SYSROOT$OSBAK/boot" -o $line == "$SYSROOT$OSBAK/usr" -o \
        $line == "$SYSROOT$OSBAK/etc" -o \
        $line == "$SYSROOT$OSBAK/var" ]; then
            continue
        else
            rsync -a -I "$line" "$SYSROOT"
        fi

        if [ $? -ne 0 ];then
            log_error -s "Rollback $line failed."
            sync
            return $ROLLBACK_ERROR
        fi
        log_info -s "Rollback $line success"
    sync
    done

    rollback_key_files
    if [ $? -ne 0 ]; then
        return $ROLLBACK_ERROR
    fi

    #set_file_caps
    if [ $? -ne 0 ];then
        log_error -s "failed to set file caps"
        return $ROLLBACK_ERROR
    fi

    is_need_selinux_relabel
    if [ $? -eq 0 ]; then
        touch $SYSROOT/.autorelabel
        if [ ! -f "$SYSROOT/.autorelabel" ]; then
            log_error -s ".autorelabel create failed"
            return $ROLLBACK_ERROR
        fi
    fi
    sync

    mkdir -p $SYSROOT/opt/ks-ssr/ks-ssr/
    touch $ROLLBACK_SUCCESS_FLAG
    if [ $? -ne 0 ]; then
        log_error -s "touch $ROLLBACK_SUCCESS_FLAG failed."
        return $COMMAND_ERROR
    fi
    chmod 0600 $ROLLBACK_SUCCESS_FLAG
    sync

    if [ ! -d $SYSROOT/var/log/kylinsec/ks-ssr/ ];then mkdir -p $SYSROOT/var/log/kylinsec/ks-ssr/; fi
    echo "ROLLBACK_SUCCESS" > $SYSROOT/var/log/kylinsec/ks-ssr/ks-ssr-stage

    return $SUCCESS
}

function is_need_selinux_relabel()
{
    local conf_path=""

    conf_path="$SYSROOT/etc/selinux/config"
    if [ ! -f "$conf_path" ]; then
        log_info -s "can not find $conf_path (ignored)"
        return $FILE_NOT_FOUND
    fi

    while read line
    do
        [ -z "$line" ] && continue
        echo "$line" | grep "^#" > /dev/null 2>&1
        [ $? -eq 0 ] && continue

        # match key=value
        key=$(echo $line|awk -F '=' '{print $1}'|awk '{gsub(/^\s+|\s+$/, "");print}')
        if [ -z "$key" ]; then
            log_error -s "selinux config read failed, key can not be null."
            return $INVALID_CONFIG
        fi

        if [ "x$key" == "xSELINUX" ]; then
            value=$(echo $line | awk -F '=' '{print $2}' | awk '{gsub(/^\s+|\s+$/, "");print}')
            if [ -z "$value" ]; then
                log_error -s "SELINUX value can not be null."
                return $INVALID_CONFIG
            fi

            if [ "x$value" == "xenforcing" ] || [ "x$value" == "xEnforcing" ]; then
            {
                sed -i "s/SELINUX=$value/SELINUX=permissive/g" $SYSROOT/etc/selinux/config > /dev/null 2>&1
                if [ $? -ne 0 ]; then
                    log_error -s "selinux state can't be modified to permissive"
                    return $COMMAND_ERROR
                fi
                sync
            }
            elif [ "x$value" == "xpermissive" ] || [ "x$value" == "xPermissive" ]; then
            {
                log_info -s "selinux state is permissive"
            }
            else
            {
                log_info -s "selinux state is $value, don't need relabel"
                return $PARAMS_ERROR
            }
            fi

            log_info -s "selinux need to do relabel"
            return $SUCCESS
        fi
    done < "$conf_path"

    log_error -s "can not get selinux state, don't need relabel"
    return $PARAMS_ERROR
}

function set_file_caps()
{
    local check_result_path=""

    check_result_path="$SYSROOT$FILE_CAP_RESULT"
    if [ ! -f "$check_result_path" ];then
        log_error -s "file check_result is not exist"
        return $FILE_NOT_FOUND
    fi

    while read line
    do
        [ -z "$line" ] && continue
        echo "$line" | grep "^\/" > /dev/null 2>&1
        if [ $? -eq 0 ];then
            files=$(echo $line|awk '{print $1}'|awk '{gsub(/^\s+|\s+$/, "");print}')
            caps=$(echo $line|awk '{print $NF}'|awk '{gsub(/^\s+|\s+$/, "");print}')
            setcap $caps $SYSROOT$files > /dev/null 2>&1
            if [ $? -eq 0 ]; then
                log_info -s "setcap $caps for $SYSROOT$files done."
            fi
        fi
    done < "$check_result_path"

    return $SUCCESS
}
################################# UPGRADE ##################################

function do_preupgrade()
{
    local hookdir=$1

    if [ ! -d "$hookdir" ];then
        log_error "Hook directory is not exist."
        return $FILE_NOT_FOUND
    fi

    if [ "$(ls -A $hookdir)" ];then
        push $hookdir &>/dev/null
        for script in $hookdir/*
        do
            chmod u+x $script &>/dev/null
            dos2unix $script &>/dev/null
            /bin/bash $script "$@" 2>&1
            if [ $? -ne 0 ];then
                log_error "Failed to execute $script in preupgrade."
                popd &>/dev/null
                return $COMMAND_ERROR
            else
                log_info "Execute $scrpt success."
            fi
        done
        popd &>/dev/null
    else
        return $SUCCESS
    fi
    return $SUCCESS
}

function do_upgrade()
{

    log_info "Execute pre upgrade hooks."
    do_preupgrade $SYSROOT$PREUPGRADEDIR
    if [ $? -ne 0 ];then
        log_error -s "Failed to execute hooks in preupgrade."
        return $COMMAND_ERROR
    fi

    log_info "Actually execute system upgrade."
    /usr/sbin/chroot $SYSROOT /usr/bin/bash -c "bash /opt/ks-ssr/ks-ssr/do_upgrade.sh"
    if [ "$?" -ne 0 ];then
        log_warn -s "Failed to upgrade system,rollback the system. "
        do_rollback --force
        if [ $? -ne 0 ];then
            log_error -s "Failed to rollback the system."
            return $ROLLBACK_ERROR
        fi
    fi
    # ensure everything is written to disk
    sync

    if [ -f /sys/fs/selinux/enforce ]; then
        echo 1 > /sys/fs/selinux/enforce
    fi
    return $SUCCESS
}

################################# MOUNT ####################################

function mount_bind()
{
    local mount_bind_dirs=("/dev" "/sys" "/proc")

    for mount_point in ${mount_bind_dirs[@]}
    do
        mount | grep " $SYSROOT$mount_point " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            log_info -s "mount_point $SYSROOT$mount_point doesn't need bind."
        else
            mount --bind $mount_point $SYSROOT$mount_point
        fi
    done

    /usr/sbin/chroot $SYSROOT /bin/bash -c "mount -a"
}

function mount_fstab_devs()
{
    local mode=$1
    local fstab_path=$SYSROOT/etc/fstab
    local device mount_point filesystem opts dump pass rest

    if [ ! -f "$fstab_path" ]; then
        log_error -s "fstab $fstab_path not exist."
        return $FILE_NOT_FOUND
    fi

    while read device mount_point filesystem opts dump pass rest; do
        [ -z "${device%%#*}" ] && continue
        [ "x$mount_point" = "xswap" ] && continue
        [ "x$mount_point" = "x/" ] && continue
        [ "x$mount_point" = "xnone" ] && continue
        part_mount "$device" "$SYSROOT$mount_point" rw
        if [ $? -ne 0 ]; then
            log_error -s "mount $device on $SYSROOT$mount_point failed."
            return $MOUNT_ERROR
        fi

        remount_efi_for_rescue $mode $mount_point
        if [ $? -ne 0 ]; then
            log_error -s "remount $device on $SYSROOT$mount_point failed."
            return $MOUNT_ERROR
        fi
    done < "$fstab_path"

    log_info -s "mount fstab devs done."

    return $SUCCESS
}

function check_fstab_devs()
{
    local mode=$1
    local fstab_path=$SYSROOT/etc/fstab
    local dev mount_dir file_system opt dump pass rest

    while read dev mount_dir file_system opt dump pass rest; do
        [ -z "${dev%%#*}" ] && continue
        [ "x$mount_dir" = "xswap" ] && continue
        [ "x$mount_dir" = "x/" ] && continue
        [ "x$mount_dir" = "xnone" ] && continue

    # grep " /opt " make sure we find excat mount_point in /proc/mounts
        mount | grep " `readlink -f $SYSROOT$mount_dir` " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            continue
        else
            part_mount "$dev" "$SYSROOT$mount_dir" rw
            if [ $? -ne 0 ]; then
                log_error -s "mount $dev on $SYSROOT$mount_dir failed in check fstab."
                return $MOUNT_ERROR
            fi
        fi

        remount_efi_for_rescue $mode $mount_dir
        if [ $? -ne 0 ]; then
            log_error -s "remount failed in check fstab."
            return $MOUNT_ERROR
        fi
    done < "$fstab_path"

    log_info -s "check fstab devs done."

    return $SUCCESS
}

function remount_efi_for_rescue()
{
    local mode=$1
    local mount_point=$2

    if [ "x$mode" == "xrescue" ] && [ "$mount_point" == "/boot/efi" ]; then
        mount | grep " `readlink -f $SYSROOT$mount_point` " > /dev/null 2>&1
        if [ $? -eq 0 ]; then
            umount "$SYSROOT$mount_point" > /dev/null 2>&1
            if [ $? -ne 0 ]; then
                log_error -s "umount $SYSROOT$mount_point failed for rescue."
                return $MOUNT_ERROR
            fi
            mount -o rw,errors=continue "$device" "$SYSROOT$mount_point"
            if [ $? -ne 0 ]; then
                log_error -s "mount $device on $SYSROOT$mount_point failed for rescue."
                return $MOUNT_ERROR
            fi
            log_info -s "mount $SYSROOT$mount_point as rw,errors=continue done for rescue."
        fi
    fi

    return $SUCCESS
}

function part_mount()
{
    local dev_tmp=$1
    local mount_point=$2
    local mount_type=$3

    if [ ! -d "$mount_point" ]; then
        log_info -s "mount_point $mount_point not found, skip."
        return $SUCCESS
    fi

    dev_tmp=`dev_transform "$dev_tmp"`
    if [ -d "$dev_tmp" ] || [ -d "$SYSROOT$dev_tmp" ] || [ ! -e "$dev_tmp" ]; then
        log_info -s "dev $dev_tmp is not need to be mounted, skip."
        return $SUCCESS
    fi

    mount | grep " `readlink -f $mount_point` " > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        mount -o remount,${mount_type} "$dev_tmp" "$mount_point"
        if [ $? -ne 0 ]; then
            log_info -s "remount error, start to fsck $dev_tmp"
            /usr/lib/systemd/systemd-fsck "$dev_tmp" > /dev/null 2>&1
            mount -o remount,${mount_type} "$dev_tmp" "$mount_point"
            if [ $? -ne 0 ]; then
                log_error -s "remount "$mount_point" failed."
                return $MOUNT_ERROR
            fi
        fi
        log_info -s "remount $dev_tmp on $mount_point as $mount_type done."
        return $SUCCESS
    fi

    mount "$dev_tmp" "$mount_point" -$mount_type > /dev/null 2>&1
    if [ $? -ne 0 ]; then
        log_info -s "mount error, start to fsck $dev_tmp"
        /usr/lib/systemd/systemd-fsck "$dev_tmp" > /dev/null 2>&1
        mount "$dev_tmp" "$mount_point" -$mount_type > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            log_error -s "mount $dev_tmp on $mount_point as $mount_type failed."
            return $MOUNT_ERROR
        fi
    fi
    log_info -s "mount $dev_tmp on $mount_point as $mount_type done."

    return $SUCCESS
}

function dev_transform()
{
    local dev_src=$1
    local dev_dst=""

    case "$dev_src" in
        LABEL=*)
            dev_src=$(echo "$dev_src" | tr -d '"')
            dev_dst="/dev/disk/by-label/${dev_src#LABEL=}"
            ;;
        UUID=*)
            dev_src=$(echo "$dev_src" | tr -d '"')
            dev_dst="/dev/disk/by-uuid/${dev_src#UUID=}"
            ;;
        PARTLABEL=*)
            dev_src=$(echo "$dev_src" | tr -d '"')
            dev_dst="/dev/disk/by-partlabel/${dev_src#PARTLABEL=}"
            ;;
        PARTUUID=*)
            dev_src=$(echo "$dev_src" | tr -d '"')
            dev_dst="/dev/disk/by-partuuid/${dev_src#PARTUUID=}"
            ;;
        *)
            dev_dst="$dev_src"
            ;;
    esac
    echo "$dev_dst"
}

################################# MAIN #################################

function main()
{
    umask $UMASK

    local cmd="$1"
    local parameter="$2"
    local root_path="$3"
    local retval=""

    case $cmd in
        upgrade)
            shift
            do_prepare "upgrade"
            if [ $? -ne 0 ]; then
                log_error -s "do prepare failed before upgrade."
                return $COMMAND_ERROR
            fi

            do_upgrade
            ;;

        rollback)
            shift
            do_prepare "rollback"
            if [ $? -ne 0 ]; then
                log_error -s "do prepare failed before rollback."
                return $COMMAND_ERROR
            fi

            do_rollback $parameter
            retval="$?"
            if [ "${retval}" -eq 0 ];then
                rm -rf "$KS_SSR_LOCK" > /dev/null 2>&1
                /usr/sbin/chroot $SYSROOT /bin/bash -c "bash /opt/ks-ssr/ks-ssr/rebuild_grub.sh"
                sync
            fi
            return "${retval}"
            ;;

        rescue)
            shift
            do_prepare "rescue" $root_path
            if [ $? -ne 0 ]; then
                log_error -s "do prepare failed before rescue system."
                return $COMMAND_ERROR
            fi

            do_rollback $parameter
            retval="$?"
            if [ "${retval}" -eq 0 ]; then
                rm -rf "$KS_SSR_LOCK" > /dev/null 2>&1
                /usr/sbin/chroot $root_path /bin/bash -c "bash /opt/ks-ssr/ks-ssr/rebuild_grub.sh"
                sync
            fi
            return "${retval}"
            ;;

        -h |--help)
            shift
            usage
            ;;

        *)
            log_error -s "Parameter error."
            usage
            return $FILE_NOT_FOUND
            ;;
    esac
}

source $WORK_DIR/global_env_variable.sh
source $WORK_DIR/log.sh

main "$@"
ret=$?
rm -rf "$KS_SSR_LOCK" > /dev/null 2>&1
exit "$ret"
