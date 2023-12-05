find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)

# ini files
file(GLOB INI_IN_FILES ./*.ini.in)

foreach(INI_IN_FILE IN LISTS INI_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" INI_FILE ${INI_IN_FILE})
  execute_process(COMMAND ${INTLTOOL-MERGE} -d ${PROJECT_SOURCE_DIR}/po/
                          ${INI_IN_FILE} ${PROJECT_BINARY_DIR}/data/${INI_FILE})
endforeach()

install(FILES ${PROJECT_BINARY_DIR}/data/br-categories.ini
              ${PROJECT_SOURCE_DIR}/data/br-public.key
        DESTINATION ${SSR_INSTALL_DATADIR}/)

# service files
if(USE_SYSTEMD)
  set(SYSTEM_UNIT_DIR "/usr/lib/systemd/system")
  pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
  install(FILES ${PROJECT_BINARY_DIR}/data/services/ks-ssr-daemon.service
          DESTINATION ${SYSTEM_UNIT_DIR})
  set(SYSTEMD_SERVICE "SystemdService=ks-ssr-daemon.service")
endif()

file(GLOB SERVICE_IN_FILES ./services/*.service.in)
foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
  configure_file(${SERVICE_IN_FILE}
                 ${PROJECT_BINARY_DIR}/data/services/${SERVICE_FILE})
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
  COMMAND sed -i -e 's/xml:lang/lang/g'
          ${PROJECT_BINARY_DIR}/data/br-system-rs.xml
  COMMAND
    ${PROJECT_BINARY_DIR}/src/tool/crypto/ks-br-crypto
    --encrypt-file=${PROJECT_BINARY_DIR}/data/br-system-rs.xml
    --private-key=${PROJECT_SOURCE_DIR}/data/br-private.key
    --output-file=${PROJECT_BINARY_DIR}/data/br-system-rs.encrypted
  DEPENDS ks-br-crypto)

install(
  FILES ${PROJECT_BINARY_DIR}/data/br-system-rs.encrypted
  DESTINATION ${SSR_INSTALL_DATADIR}/
  RENAME br-system-rs)

install(
  FILES ${PROJECT_BINARY_DIR}/data/services/com.kylinsec.SSR.service
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

# ini files
install(FILES ${PROJECT_SOURCE_DIR}/data/ssr.ini
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

# desktop
configure_file(${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.desktop.in
               ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop)

install(FILES ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/)

# appdata.xml
install(FILES ${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.appdata.xml
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo/)

# selinux mode files
install(
  FILES ${PROJECT_SOURCE_DIR}/data/br-sshd-port.pp
        ${PROJECT_SOURCE_DIR}/data/br-sshd-sftp.pp
        ${PROJECT_SOURCE_DIR}/data/br-ulimit.pp
  DESTINATION ${SSR_INSTALL_DATADIR}/)

# iptables rules files
install(FILES ${PROJECT_SOURCE_DIR}/data/br-iptables.rules
        DESTINATION ${SSR_INSTALL_DATADIR}/)
