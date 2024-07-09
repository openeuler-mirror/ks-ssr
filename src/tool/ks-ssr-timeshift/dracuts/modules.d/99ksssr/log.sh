#!/bin/bash
# Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
# Description: This script contains the MiniOS Log interfaces and generic
#              utility functions.
# 

#const value of different loglevel
CON_DEBUG="1"
CON_INFO="2"
CON_ERROR="3"
CON_WARN="4"

Default_Level="2"
LOG_LEVEL=$Default_Level

function Ipu_WriteLog()
{
    local severity=$1
    local line_no=$2
    shift 2

    local stdout=$1
    local stdout_flag="false"
    if [ "x$stdout" = "x-s" ]; then
        shift 1
        stdout_flag="true"
    fi

    local progname="[initramfs]"
    local script=`basename "${BASH_SOURCE[2]}"`
    local logmsg=$@
    local date_time=$(date "+%Y-%m-%d %H:%M:%S,%3N")
    declare -A map=([1]="DEBUG" [2]="INFO" [3]="ERROR" [4]="WARN")
    if [ "x$stdout_flag" = "xtrue" ]; then
        echo "[ ${map[$severity]} ] - ${progname}: ${logmsg}"
    fi
    if [ -d "$SYSROOT$LOG_PATH" ]; then
        echo "${date_time} [ ${map[$severity]} ] - ${progname} [${script}] [line=${line_no}]: ${logmsg}" >> $SYSROOT$LOG_PATH/$KS_SSR_LOG
    fi
}

function log_error()
{
    Ipu_WriteLog $CON_ERROR $BASH_LINENO "$@"
}

function log_warn()
{
    Ipu_WriteLog $CON_WARN $BASH_LINENO "$@"
}

function log_info()
{
    Ipu_WriteLog $CON_INFO $BASH_LINENO "$@"
}

function log_debug()
{
    Ipu_WriteLog $CON_DEBUG $BASH_LINENO "$@"
}
