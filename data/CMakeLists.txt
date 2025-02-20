find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)

# 此功能用于客户使用， 打包时获取 cve 元数据并打进包中， 允许用户在不配源的情况下进行漏洞扫描。
if(ENABLE_VULNERABILITY_MANAGER AND ENABLE_DEFAULT_CVE_REPO)
  # 查找 wget 命令
  find_program(WGET NAMES "wget" REQUIRED)
  message(
    STATUS
      "execute process : wget -r -c -nH -np -R \"index*\"  http://192.168.120.89/repos/CentOS7.9/x86_64/os/x86_64/repodata/"
  )
  # 在 cmake 中执行命令 wget -r -c -nH -np -R "index*"
  # http://192.168.120.89/repos/CentOS7.9/x86_64/updates/x86_64/repodata/
  execute_process(
    COMMAND
      ${WGET} -q -r -c -nH -np -nd -R "index*"
      http://192.168.120.89/repos/CentOS7.9/x86_64/os/x86_64/repodata/ -P
      default-repos/os/x86_64/repodata
    RESULT_VARIABLE result
    ERROR_VARIABLE error_var)
  # 判断 result 是否不等于 0
  if(result)
    message(
      WARNING
        "Failed to wget default Vulnerability repo with error: ${error_var}")
  endif()

  message(
    STATUS
      "execute process : wget -r -c -nH -np -R \"index*\"  http://192.168.120.89/repos/CentOS7.9/x86_64/updates/x86_64/repodata/"
  )
  execute_process(
    COMMAND
      ${WGET} -q -r -c -nH -np -nd -R "index*"
      http://192.168.120.89/repos/CentOS7.9/x86_64/updates/x86_64/repodata/ -P
      default-repos/updates/x86_64/repodata
    RESULT_VARIABLE result
    ERROR_VARIABLE error_var)
  if(result)
    message(
      WARNING
        "Failed to wget default Vulnerability repo with error: ${error_var}")
  endif()

  configure_file(${CMAKE_SOURCE_DIR}/data/vulnerability/default.repo.in
                 ${CMAKE_BINARY_DIR}/data/vulnerability/default.repo)

  install(DIRECTORY ${CMAKE_BINARY_DIR}/default-repos
          DESTINATION ${SSR_VULNERABILITY_INSTALL_DATADIR}/)

  install(FILES ${CMAKE_BINARY_DIR}/data/vulnerability/default.repo
          DESTINATION ${SSR_VULNERABILITY_INSTALL_DATADIR}/yum.repos.d/)
endif()
# ini files
file(GLOB INI_IN_FILES ./*.ini.in)

foreach(INI_IN_FILE IN LISTS INI_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" INI_FILE ${INI_IN_FILE})
  execute_process(COMMAND ${INTLTOOL-MERGE} -d ${PROJECT_SOURCE_DIR}/po/
                          ${INI_IN_FILE} ${PROJECT_BINARY_DIR}/data/${INI_FILE})
endforeach()

install(FILES ${PROJECT_SOURCE_DIR}/data/br-categories.ini
              ${PROJECT_SOURCE_DIR}/data/br-public.key
        DESTINATION ${SSR_BR_INSTALL_DATADIR}/)

# systemd service files
install(FILES ${PROJECT_BINARY_DIR}/data/services/systemd/ks-ssr-daemon.service
        DESTINATION ${SYSTEM_UNIT_DIR})

# initd service files
install(
  FILES ${PROJECT_SOURCE_DIR}/data/services/initd/ks-ssr-daemon
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
  DESTINATION ${INITD_SERVICE_DIR})

if(ENABLE_TOOL_BOX)
  set(SELINUX_CONTEXT
      "SELinuxContext=unconfined_u:unconfined_r:unconfined_t:s0-s15:c0.c1023")
  set(IS_REFUSE_MANUAL_STOP "RefuseManualStop=yes")
endif()

if(ENABLE_ACTIVATION)
  set(SYSTEM_SERVICE_DEP "ksl-daemon.service ")
endif()

file(GLOB SERVICE_IN_FILES ./services/systemd/*.service.in)
foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
  configure_file(${SERVICE_IN_FILE}
                 ${PROJECT_BINARY_DIR}/data/services/systemd/${SERVICE_FILE})
endforeach()

# xml files
file(GLOB XML_IN_FILES ./*.xml.in)

foreach(XML_IN_FILE IN LISTS XML_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" XML_FILE ${XML_IN_FILE})
  execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/
                          ${XML_IN_FILE} ${PROJECT_BINARY_DIR}/data/${XML_FILE})
endforeach()

add_custom_target(
  br-system-rs.encrypted ALL
  COMMAND
    ${PROJECT_BINARY_DIR}/src/tool/crypto/ks-br-crypto
    --encrypt-file=${PROJECT_SOURCE_DIR}/data/${BR_RS_CONFIG}
    --private-key=${PROJECT_SOURCE_DIR}/data/br-private.key
    --output-file=${PROJECT_BINARY_DIR}/data/br-system-rs.encrypted
  DEPENDS ks-br-crypto)

install(
  FILES ${PROJECT_BINARY_DIR}/data/br-system-rs.encrypted
  DESTINATION ${SSR_BR_INSTALL_DATADIR}/
  RENAME br-system-rs)

install(
  FILES ${PROJECT_BINARY_DIR}/data/services/systemd/com.kylinsec.SSR.service
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

# policy file
install(
  FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.SSR.policy.in
  RENAME com.kylinsec.SSR.policy
  DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions/)

# conf files
install(FILES ${PROJECT_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.conf
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/)

# box dir
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/box
        DESTINATION ${SSR_INSTALL_DATADIR})

# configuration files
install(
  FILES ${PROJECT_SOURCE_DIR}/data/ssr.ini ${PROJECT_SOURCE_DIR}/data/zlog.conf
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
              WORLD_WRITE
  DESTINATION ${SSR_INSTALL_DATADIR})

# device cfg file
install(
  FILES ${PROJECT_SOURCE_DIR}/data/device-config.ini
        ${PROJECT_SOURCE_DIR}/data/device-interface-config.ini
        ${PROJECT_SOURCE_DIR}/data/device-connect-record.ini
  DESTINATION ${SSR_INSTALL_DATADIR}/dm)

# udev cfg file
install(FILES ${PROJECT_SOURCE_DIR}/data/99-ssr-device.rules
        DESTINATION /etc/udev/rules.d/)

# audit rules file
install(FILES ${PROJECT_SOURCE_DIR}/data/ks-ssr-audit.rules
        DESTINATION /etc/audit/rules.d/)

configure_file(${PROJECT_SOURCE_DIR}/data/ks-ssr-notify.desktop.in
               ${PROJECT_BINARY_DIR}/data/ks-ssr-notify.desktop)
install(FILES ${PROJECT_BINARY_DIR}/data/ks-ssr-notify.desktop
        DESTINATION /etc/xdg/autostart/)

# selinux mode files
install(
  FILES ${PROJECT_SOURCE_DIR}/data/br-sshd-port.pp
        ${PROJECT_SOURCE_DIR}/data/br-sshd-sftp.pp
        ${PROJECT_SOURCE_DIR}/data/br-ulimit.pp
  DESTINATION ${SSR_BR_INSTALL_DATADIR}/)

# iptables rules files
install(FILES ${PROJECT_SOURCE_DIR}/data/br-iptables.rules
        DESTINATION ${SSR_BR_INSTALL_DATADIR}/)

if(ENABLE_GUI)
  install(FILES ${PROJECT_SOURCE_DIR}/data/help.pdf
          DESTINATION ${SSR_INSTALL_DATADIR}/)

  # desktop
  configure_file(${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.desktop.in
                 ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop)
  install(FILES ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop
          DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/)
  # appdata.xml
  install(FILES ${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.appdata.xml
          DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo/)

  install(FILES ${PROJECT_SOURCE_DIR}/data/AlibabaPuHuiTi-3-55-Regular.ttf
          DESTINATION ${SSR_INSTALL_DATADIR}/)
endif()
