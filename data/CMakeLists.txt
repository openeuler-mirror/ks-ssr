

find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)


# ini files
file(GLOB INI_IN_FILES ./*.ini.in)

foreach(INI_IN_FILE IN LISTS INI_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" INI_FILE ${INI_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -d ${PROJECT_SOURCE_DIR}/po/ ${INI_IN_FILE} ${PROJECT_BINARY_DIR}/data/${INI_FILE})
endforeach()


install(FILES ${PROJECT_SOURCE_DIR}/data/sse.ini
              ${PROJECT_BINARY_DIR}/data/sse-categories.ini
              ${PROJECT_SOURCE_DIR}/data/sse-public.key
        DESTINATION ${SSE_INSTALL_DATADIR}/)

install (FILES ${PROJECT_BINARY_DIR}/data/sse-plugin-config.ini
        DESTINATION ${SSE_PLUGIN_ROOT_DIR}/)


# reinforcement stardand
install(FILES ${PROJECT_SOURCE_DIR}/data/sse-system-rs.json.encrypted
        DESTINATION ${SSE_INSTALL_DATADIR}/
        RENAME sse-system-rs.json)

# service files
file(GLOB SERVICE_IN_FILES ./*.service.in)

foreach(SERVICE_IN_FILE IN LISTS SERVICE_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" SERVICE_FILE ${SERVICE_IN_FILE})
    configure_file(${SERVICE_IN_FILE} ${PROJECT_BINARY_DIR}/data/${SERVICE_FILE})
endforeach()

install(FILES ${PROJECT_BINARY_DIR}/data/com.kylinsec.Kiran.SSE.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)

if (SYSTEMD_FOUND)
    pkg_get_variable(SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
    install(FILES ${PROJECT_BINARY_DIR}/data/kiran-sse-daemon.service
            DESTINATION ${SYSTEM_UNIT_DIR})
endif()


# conf files
install(FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.Kiran.SSE.conf
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/)

install (FILES ${PROJECT_SOURCE_DIR}/data/10-sysctl-ssr.conf
         DESTINATION /etc/sysctl.d/)


