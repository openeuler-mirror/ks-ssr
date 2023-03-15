# service files
if (USE_SYSTEMD)
    pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
    install(FILES ${PROJECT_BINARY_DIR}/data/ks-sc-daemon.service
            DESTINATION ${SYSTEM_UNIT_DIR})
    set(SYSTEMD_SERVICE "SystemdService=ks-sc-daemon.service")
endif()

file(GLOB SERVICE_IN_FILES ./*.service.in)
foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
    configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
endforeach()

install(FILES ${PROJECT_BINARY_DIR}/data/com.kylinsec.SC.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

# conf files
install(FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.SC.conf
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/)

# box sql
install(FILES ${PROJECT_SOURCE_DIR}/data/box-dao.dat
        DESTINATION ${SC_INSTALL_DATADIR})

# box dir
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data/box
        DESTINATION ${SC_INSTALL_DATADIR})