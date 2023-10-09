# service files
if(USE_SYSTEMD)
  set(SYSTEM_UNIT_DIR "/usr/lib/systemd/system")
  pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
  install(FILES ${PROJECT_BINARY_DIR}/data/ks-ssr-daemon.service
          DESTINATION ${SYSTEM_UNIT_DIR})
  set(SYSTEMD_SERVICE "SystemdService=ks-ssr-daemon.service")
endif()

file(GLOB SERVICE_IN_FILES ./*.service.in)
foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
  configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
endforeach()

install(
  FILES ${PROJECT_BINARY_DIR}/data/com.kylinsec.SSR.service
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

# policy file
install(
  FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.SSR.policy.in
  RENAME com.kylinsec.SSR.policy
  DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/polkit-1/actions/)

# conf files
install(FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.SSR.conf
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/)

# ssr sql
install(FILES ${PROJECT_SOURCE_DIR}/data/ssr.dat
        DESTINATION ${SSR_INSTALL_DATADIR})

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

# desktop
configure_file(${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.desktop.in
               ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop)

install(FILES ${PROJECT_BINARY_DIR}/data/ks-ssr-gui.desktop
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/applications/)

# appdata.xml
install(FILES ${PROJECT_SOURCE_DIR}/data/ks-ssr-gui.appdata.xml
        DESTINATION ${CMAKE_INSTALL_FULL_DATADIR}/metainfo/)
