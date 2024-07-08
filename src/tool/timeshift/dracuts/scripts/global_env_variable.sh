#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
# Description: This script contains the MiniOS Log interfaces and generic
#              utility functions.
#
export UMASK=0077
export DNF_DIR=/root/dnf
export MOUNT_ISO_POINT=/tmp/tmp_iso
export LEGACY_BOOT=/boot/grub2
export UEFI_BOOT=/boot/efi/EFI
export DEFAULT_REPO=/etc/yum.repos.d/
export YUM_CACHE=/var/cache/yum
export KERNEL_MODULE=/usr/lib/modules/`uname -r`/kernel/
export KS_SSR_CHECK_RESULT=/opt/ks-ssr/check_result
export KS_SSR_CONF="/usr/share/ks-ssr/ssr.ini"
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
export UPGRADE_SYSTEM_BY_DNF=11
export CONTINUE_DO_UPGRADE=12
export CONNOT_DO_RETRY=13
