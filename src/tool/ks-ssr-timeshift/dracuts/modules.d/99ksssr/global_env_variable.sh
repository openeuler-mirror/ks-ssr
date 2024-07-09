#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
# Description: This script contains the MiniOS Log interfaces and generic
#              utility functions.
# 
export UMASK=0077
export SYSROOT="/sysroot"
export PREUPGRADEDIR="/opt/ks-ssr/preupgrade/"
export UEFI_BOOT="/boot/efi/EFI"
export LEGACY_BOOT="/boot/grub2"
export KS_SSR_LOCK="/ks-ssr.lock"
export OS_VERSION="$SYSROOT/etc/KylinSecOS-latest"
export OS_VERSION_CONF="$SYSROOT/etc/KylinSecLinux.conf"
export KS_SSR_CONF="$SYSROOT/usr/share/ks-ssr/ssr.ini"
export FILE_CAP_RESULT="/opt/ks-ssr/file_cap_result"
export ROLLBACK_SUCCESS_FLAG="$SYSROOT/opt/ks-ssr/rollback_flag"
export LOG_PATH="/var/log/kylinsec/ks-ssr/"
export KS_SSR_LOG="ks-ssr-rollback.log"

# Error return code
export SUCCESS=0
export FILE_NOT_FOUND=1
export COMMAND_ERROR=2
export UPGRADE_ERROR=3
export ROLLBACK_ERROR=4
export TRAP_SIG_EXIT=5
export PROCESS_ALREAD_EXIST=6
export MOUNT_ERROR=7
export NO_NEED_ROLLBACK=8
export FAILED=9
export INVALID_CONFIG=10
export PARAMS_ERROR=11
export DEV_NOT_EXIST=12

