function centos6_init(){
    initfile="/usr/share/dracut/modules.d/99base/init"
    if [ -f ${initfile} ];then
        grep -r "if\ getarg\ rollback" ${initfile} > /dev/null
        if [ $? -ne 0 ];then
        sed /getarg\ rdbreak\ \&\&\ emergency_shell\ -n\ switch_root\ \"Break\ before\ switch_root\"/a\if\ getarg\ rollback\;\ then\\n/bin/ssr-rollback\ rollback\ \-\-force\\n\/sbin\/reboot\ \-f\\nfi ${initfile} -i
        fi
    fi
}

centos6_init

