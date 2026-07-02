

find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)


# ini files
file(GLOB INI_IN_FILES ./*.ini.in)

foreach(INI_IN_FILE IN LISTS INI_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" INI_FILE ${INI_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -d ${PROJECT_SOURCE_DIR}/po/ ${INI_IN_FILE} ${PROJECT_BINARY_DIR}/data/${INI_FILE})
endforeach()

install(FILES ${PROJECT_SOURCE_DIR}/data/ssr.ini
              ${PROJECT_BINARY_DIR}/data/ssr-categories.ini
              ${PROJECT_SOURCE_DIR}/data/ssr-public.key
        DESTINATION ${SSR_INSTALL_DATADIR}/)


# xml files
file(GLOB XML_IN_FILES ./*.xml.in)

foreach(XML_IN_FILE IN LISTS XML_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" XML_FILE ${XML_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/ ${XML_IN_FILE} ${PROJECT_BINARY_DIR}/data/${XML_FILE})
endforeach()

add_custom_target(ssr-system-rs.encrypted ALL
                  COMMAND sed -i -e 's/xml:lang/lang/g' ${PROJECT_BINARY_DIR}/data/ssr-system-rs.xml
                  COMMAND ${PROJECT_BINARY_DIR}/src/tool/crypto/ks-ssr-crypto --encrypt-file=${PROJECT_BINARY_DIR}/data/ssr-system-rs.xml
                                                                              --private-key=${PROJECT_SOURCE_DIR}/data/ssr-private.key
                                                                              --output-file=${PROJECT_BINARY_DIR}/data/ssr-system-rs.encrypted
                  DEPENDS ks-ssr-crypto)

#add_custom_target(ssr-custom-rs.encrypted ALL
#                  COMMAND sed -i -e 's/xml:lang/lang/g' ${PROJECT_BINARY_DIR}/data/ssr-custom-rs.xml
#                  COMMAND ${PROJECT_BINARY_DIR}/src/tool/crypto/ks-ssr-crypto --encrypt-file=${PROJECT_BINARY_DIR}/data/ssr-custom-rs.xml
#                                                                              --private-key=${PROJECT_SOURCE_DIR}/data/ssr-private.key
#                                                                              --output-file=${PROJECT_BINARY_DIR}/data/ssr-custom-rs.encrypted
#                  DEPENDS ks-ssr-crypto)

install(FILES ${PROJECT_BINARY_DIR}/data/ssr-system-rs.encrypted
        DESTINATION ${SSR_INSTALL_DATADIR}/
        RENAME ssr-system-rs)

#install(FILES ${PROJECT_BINARY_DIR}/data/ssr-custom-ra.encrypted
#        DESTINATION ${SSR_INSTALL_DATADIR}/
#        RENAME ssr-custom-rs)

#install (FILES ${PROJECT_SOURCE_DIR}/data/ssr-custom-ra.xml
#        DESTINATION ${SSR_INSTALL_DATADIR})

#install (FILES ${PROJECT_SOURCE_DIR}/data/ssr-custom-ra-strategy.xml
#        DESTINATION ${SSR_INSTALL_DATADIR})

# service files
if (USE_SYSTEMD)
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

install(FILES ${PROJECT_BINARY_DIR}/data/com.kylinsec.SSR.service
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/dbus-1/system-services)


# conf files
install(FILES ${PROJECT_SOURCE_DIR}/data/com.kylinsec.SSR.conf
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/dbus-1/system.d/)

# pc file
file(GLOB PC_IN_FILES ./*.pc.in)

foreach(PC_IN_FILE IN LISTS PC_IN_FILES)
    string(REGEX REPLACE ".+/(.+)\\..*" "\\1" PC_FILE ${PC_IN_FILE})
    configure_file(${PC_IN_FILE} ${PROJECT_BINARY_DIR}/data/${PC_FILE})

    install(FILES ${PROJECT_BINARY_DIR}/data/${PC_FILE}
            DESTINATION ${SSR_INSTALL_LIBDIR}/pkgconfig)
endforeach()

# selinux mode files
install(FILES ${PROJECT_SOURCE_DIR}/data/ssr-sshd-port.pp
                ${PROJECT_SOURCE_DIR}/data/ssr-sshd-sftp.pp
                ${PROJECT_SOURCE_DIR}/data/ssr-ulimit.pp
        DESTINATION ${SSR_INSTALL_DATADIR}/)

