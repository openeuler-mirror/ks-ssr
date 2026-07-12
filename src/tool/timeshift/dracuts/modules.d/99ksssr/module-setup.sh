#!/bin/bash
# -*- mode: shell-script; indent-tabs-mode: nil; sh-basic-offset: 4; -*-
# ex: ts=8 sw=4 sts=4 et filetype=sh
check() {
    if [ ! -f "/opt/ks-ssr/.ks-ssr_dracut_flag" ];then
        return 1
    fi
    return 0
}

depends() {
    return 0
}

install() {
    inst_multiple rsync sync mount umount ip ifconfig chmod dos2unix date basename dirname expr touch sed setcap awk realpath
	
    if [ -f /usr/sbin/efibootmgr ];then
        inst_multiple efibootmgr
    fi

    mkdir -m 0500 -p "$initdir/opt/ks-ssr/ks-ssr"

    inst_script "$moddir/log.sh" "/opt/ks-ssr/ks-ssr/log.sh"
    inst_script "$moddir/global_env_variable.sh" "/opt/ks-ssr/ks-ssr/global_env_variable.sh"
    inst_script "$moddir/ssr-rollback.sh" "/bin/ssr-rollback"

    if [ -f "/opt/ks-ssr/.need_service" ];then
        local target=initrd-root-fs.target
        if dracut_module_included "systemd"; then
            inst_simple "$moddir/rollback.service" "$systemdsystemunitdir/rollback.service"	
            mkdir -p "$initdir/$systemdsystemunitdir/${target}.wants"
            ln_r "$systemdsystemunitdir/rollback.service" "$systemdsystemunitdir/${target}.wants/rollback.service"
        fi
    fi

    command -v lvm 2>/dev/null 1>/dev/null
    if [ $? -eq 0 ]; then
        inst_multiple lvm vgchange thin_check thin_repair
    fi
}
