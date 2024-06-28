set(TARGET_NAME ks-ssr-cmd)

pkg_search_module(XERCES REQUIRED xerces-c)

file(GLOB CMD_H_FILES ./*.h)
file(GLOB CMD_CPP_FILES ./*.cpp)

gen_protocol()

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  PROPERTIES CLASSNAME BRDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  br_dbus_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Vulnerability.xml
  PROPERTIES CLASSNAME VulnerabilityDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  Vulnerability_DBUS_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Vulnerability.xml
  vulnerability_dbus_proxy)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")

execute_process(
  COMMAND
    lupdate-qt5 -no-obsolete ${CMD_H_FILES} ${CMD_CPP_FILES}
    ${CMAKE_SOURCE_DIR}/data/br-system-rs.xml
    ${CMAKE_SOURCE_DIR}/data/br-categories.ini
    ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-audit.xml
    ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-config.xml
    ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-external.xml
    ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-network.xml -ts
    ${TS_FILES})

qt5_add_translation(QM_FILES ${TS_FILES})

add_executable(
  ${TARGET_NAME} ${CMD_H_FILES} ${CMD_CPP_FILES} ${BR_DBUS_SRCS}
                 ${Vulnerability_DBUS_SRCS} ${BR_PROTOCOL_OUTPUT} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::DBus Qt5::Core
                                             ${XERCES_LIBRARIES} base dbus)

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/lib64")

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_LIBEXECDIR})
install(
  PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/ks-ssr-cmd.sh"
  RENAME ${TARGET_NAME}
  DESTINATION ${SSR_INSTALL_BINDIR})
