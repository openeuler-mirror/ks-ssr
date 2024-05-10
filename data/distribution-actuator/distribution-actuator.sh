#!/bin/bash

#分发最大并发进程
DISTRIBUTION_MAX_PROCESS=1

#批量操作选项
OPTION_REINFORCE=0
OPTION_REPAIR=0

#加固参数
REINFORCE_ARG_COUNT=0
REINFORCE_ARGS=""

#配置信息
LOGIN_CONFIG_COUNT=0
declare -A LOGIN_CONFIGS

# 打印帮助信息
function usage() {
  echo "Usage: $0 {list|excute} <machine-config> <reinforce>|<repair>"
  exit 1
}

# 终端输出调用者能识别的消息
function feedback() {
    local cmd=$1
    local msg=$2
    printf "kylinsec_ssr_actuator_feedback: %s %s\n" "$cmd" "$msg"
}

function total_job_feedback() {
    feedback "total-job" "$LOGIN_CONFIG_COUNT"
}

function entry_complete_feedback() {
    local address=$1
    local result=$2

    feedback "job-done" "$address,$result"
}

function debug_feedback() {
    local address=$1
    local msg=$2

    feedback "debug" "$address,$msg"
}

# 退出清理
function abnormal_signal_handle() {
    exit 1;
}

function dispatch_clean_on_exit() {
    echo "dispatch clean on exit"

    kill $(jobs -p) 2>/dev/null || true

    exec 1000>&-;
    exec 1000<&-;
}

# 检查并提取机器配置文件行内容
# : 1. 配置文件路径
# : 2. 返回配置内的ip
# : 3. 返回配置内的passwd
function check_machine_line() {
    if [ $# -ne 3 ]; then
        return 1
    fi

    local machine_line=$1
    local -n ip_ref=$2
    local -n passwd_ref=$3

    ip_ref=$( echo $machine_line | awk -F',' '{print $1}' )
    passwd_ref=$( echo $machine_line | awk -F',' '{print $2}' )
    if [[ -z "$ip_ref" || -z "$passwd_ref" ]]; then
        return 1
    fi

    return 0
}

# 从配置中加载并初始化全局变量
# : 1. 配置文件路径
function load_login_config() {
    local machine_config=$1

    if [[ ! -f "$machine_config" ]]; then
        echo "machine config not exit"
        exit 1
    fi

    while read -r line; do
        check_machine_line $line ip passwd
        if [[ $? -ne 0 ]]; then
            continue
        fi
        LOGIN_CONFIGS[$ip]="$passwd"
    done < "$machine_config"

    LOGIN_CONFIG_COUNT=${#LOGIN_CONFIGS[@]}

    echo "login config entry: ${#LOGIN_CONFIGS[@]}"
    echo "address: ${!LOGIN_CONFIGS[@]}"
    echo "passwd:  ${LOGIN_CONFIGS[@]}"
}

declare -A SSHPASS_ERRORS=()
SSHPASS_ERRORS[1]="Invalid command line argument"
SSHPASS_ERRORS[2]="Conflicting arguments given"
SSHPASS_ERRORS[3]="General runtime error"
SSHPASS_ERRORS[4]="Unrecognized response from ssh (parse error)"
SSHPASS_ERRORS[5]="Invalid/incorrect password"
SSHPASS_ERRORS[6]="Host public key is unknown. sshpass exits without confirming the new key"
SSHPASS_ERRORS[7]="IP public key changed. sshpass exits without confirming the new key"
# 解析sshpass的错误码
# : 1. 错误码
# : 2. 返回的错误信息
function sshpass_error() {
    local ret=$1
    local -n msg_ref=$2

    if [[ -v SSHPASS_ERRORS[$ret] ]]; then
        msg_ref="${SSHPASS_ERRORS[$ret]}"
    else
        msg_ref="Unknown error"
    fi

    return 0
}

# 加载加固参数并初始化全局变量
function load_reinforce_args() {
    REINFORCE_ARG_COUNT=$( wc -l < ./Reinforce.args )
    ((REINFORCE_ARG_COUNT++))
    REINFORCE_ARGS=$(tr '\n' ' ' < ./Reinforce.args)
}

# 远程执行命令
# ：1. address:远程地址
# ：2. passwd:远程密码
# ：3. cmds:远程命令
function remote_excute() {
    local address=$1
    local passwd=$2
    local cmds=$3

    sshpass -p "$passwd" ssh -o StrictHostKeyChecking=no -o ConnectTimeout=2 root@"$address" 1>/dev/null 2>&1 << EOF
        $cmds  >/tmp/remote_excute.log 2>&1
EOF

    local ret=$?
    if [[ $ret -ne 0 ]]; then
        sshpass_error $ret msg
        debug_feedback $address "远程执行命令：$cmds 错误：$msg"
    fi

    return $ret
}

# 检查机器SSR安装情况
function check_ssr_install() {
    local address=$1
    local passwd=$2

    local check_cmd=""
    check_cmd="rpm -qi ks-ssr-manager"
    remote_excute "$address" "$passwd" "$check_cmd"

    return $?
}

#TODO: 完善安装包分发以及安装
# 分发SSR安装包并安装
function dispatch_package_and_install() {
    local address=$1
    local passwd=$2

    sshpass -p "$passwd" scp -o StrictHostKeyChecking=no -o ConnectTimeout=2 /var/log/messages root@"$address":/tmp/ks-ssr-manager.rpm
    ret=$?
    if [ $ret -ne 0 ]; then
        return $ret
    fi

    local install_cmd="sleep 1"
    remote_excute "$address" "$passwd" "$install_cmd"

    return $?
}

# 检查远程机器上SSR服务是否运行
function check_ssr_running() {
    local address=$1
    local passwd=$2

    local check_cmd=""
    check_cmd="systemctl status ks-ssr-daemon"
    remote_excute "$address" "$passwd" "$check_cmd"

    return $?
}

# 调用机器远程加固
function reinforce() {
    local address=$1
    local passwd=$2

    local reinforce_cmd=""
    reinforce_cmd="busctl --system call com.kylinsec.SSR /com/kylinsec/SSR com.kylinsec.SSR Reinforce \"as\" $REINFORCE_ARG_COUNT $REINFORCE_ARGS"
    remote_excute "$address" "$passwd" "$reinforce_cmd"
    return $?
}

function repair() {
    local address=$1
    local passwd=$2

    local repair_cmd=""
    repair_cmd="busctl --system call com.kylinsec.SSR /com/kylinsec/SSR/Vulnerability com.kylinsec.SSR.Vulnerability UpgradeAllPkg"
    remote_excute "$address" "$passwd" "$repair_cmd"
    return $?
}


# 单个机器执行
function excute() {
    local address=$1
    local passwd=$2

    debug_feedback "$address" "开始进行加固..."

    if ! check_ssr_install "$address" "$passwd"; then
        debug_feedback "$address" "SSR Not Installed"
        return 1
        # if ! dispatch_package_and_install "$address" "$passwd"; then
        #     debug_feedback "$address" "Dispatch Package Failed"
        #     return 1
        # fi
    fi
    debug_feedback "$address" "安全加固已安装"

    if ! check_ssr_running "$address" "$passwd"; then
        debug_feedback "$address" "安全加固未运行"
        return 1
    fi
    debug_feedback "$address" "安全加固已运行"

    if [[ $OPTION_REINFORCE == 1 ]]; then
        if ! reinforce "$address" "$passwd"; then
            debug_feedback "$address" "安全加固执行失败"
            return 1
        fi
        debug_feedback "$address" "已完成基线加固"
    fi

    if [[ $OPTION_REPAIR == 1 ]]; then
        if ! check_ssr_repair "$address" "$passwd"; then
            debug_feedback "$address" "漏洞修复失败"
            return 1
        fi
        debug_feedback "$address" "已完成漏洞修复"
    fi

    sleep 1
    return 0
}

# 多进程分发执行任务
function dispatch() {
    load_reinforce_args
    total_job_feedback

    # 创建管道，绑定文件描述符，删除管道
    # 用来限制后台同时运行进程数量
    local fifo_name="/tmp/process_limit.fifo"
    mkfifo $fifo_name
    exec 1000<>$fifo_name
    rm -f $fifo_name

    trap 'dispatch_clean_on_exit' EXIT
    trap 'abnormal_signal_handle' SIGINT SIGTERM

    # 对文件操作符进行写入操作。
    # 通过一个for循环写入<n>个空行，这个n就是我们要定义的后台线程数量。
    for ((n=0; n<DISTRIBUTION_MAX_PROCESS; n++))
    do
        echo >&1000
    done

    # 读取机器配置信息，分发执行后台任务
    for ip in "${!LOGIN_CONFIGS[@]}"; do
        passwd=${LOGIN_CONFIGS[$ip]}
        {
            read -u1000
            excute $ip $passwd
            entry_complete_feedback  "$ip" "$?"
            echo >&1000
        }&
    done

    wait
}

# __main__
if [[ $# -lt 2 ]]; then
    usage
fi

cmd=$1
machine_config=$2

for arg in "$@"; do
    if [[ "$arg" == "reinforce" ]];then
        OPTION_REINFORCE=true
        echo "enable reinforce.."
    fi
    if [[ "$arg" == "repair" ]];then
        OPTION_REPAIR=true
        echo "enable repair.."
    fi
done

for ((i=0;i<=$#;i++))
do
    if [[ "${!i}" == "reinforce" ]];then
        OPTION_REINFORCE=1
    elif [[ "${!i}" == "repair" ]];then
        OPTION_REPAIR=1
        echo "enable repair.."
    fi
done

load_login_config "$machine_config"
case "$cmd" in
    list)
    feedback "list" "$LOGIN_CONFIG_COUNT"
    ;;
    excute)
    if [[ $OPTION_REINFORCE -eq 0 && $OPTION_REPAIR -eq 0 ]];then
        exit 1
    fi
    dispatch "$machine_config"
    ;;
    *)
    usage
    ;;
esac

exit 0
