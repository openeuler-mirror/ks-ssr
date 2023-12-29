cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

file(GLOB_RECURSE UI_H_FILES ./*.h)
file(GLOB_RECURSE UI_CPP_FILES ./*.cpp)

configure_file(config-ui.h.in ${PROJECT_BINARY_DIR}/config-ui.h)

gen_protocol()

set_source_files_properties(${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml
                            PROPERTIES CLASSNAME DaemonProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  DAEMON_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml daemon_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
  PROPERTIES CLASSNAME BoxManagerProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
  box_manager_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
  PROPERTIES CLASSNAME KSSDbusProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
  kss_dbus_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
  PROPERTIES CLASSNAME DeviceManagerProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
  device_manager_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  PROPERTIES CLASSNAME BRDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  br_dbus_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml
  PROPERTIES CLASSNAME LogProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  LOG_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml log_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  PROPERTIES CLASSNAME ToolBoxDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  toolbox_dbus_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
  PROPERTIES CLASSNAME AccountProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  ACCOUNT_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
  account_proxy)

qt5_wrap_ui(
  UI_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

set(TARGET_NAME ks-ssr-gui)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt5_create_translation(
  QM_FILES
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/data/br-system-rs.xml.in
  ${CMAKE_SOURCE_DIR}/data/br-categories.ini.in
  ${CMAKE_SOURCE_DIR}/plugins/python/br-plugin-audit.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/python/br-plugin-config.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/python/br-plugin-external.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/python/br-plugin-network.xml.in
  ${TS_FILES}
  OPTIONS
  -I
  ${PROJECT_SOURCE_DIR})

set(RESOURCE_LOGO_PNG_PATH
    "${PROJECT_SOURCE_DIR}/resources/images/kylinsec-logo.png")
set(RESOURCE_REPORTS_END_PNG_PATH
    "${PROJECT_SOURCE_DIR}/resources/images/PDF/end.png")

# 判断private目录是否拉取，替换logo图片
if(EXISTS "${PROJECT_SOURCE_DIR}/private/check-project-exist")
  execute_process(
    COMMAND
      ln -s ${PROJECT_SOURCE_DIR}/private/resources/images/kylinsec-logo.png
      ${RESOURCE_LOGO_PNG_PATH}
    COMMAND ln -s ${PROJECT_SOURCE_DIR}/private/resources/images/PDF/end.png
            ${RESOURCE_REPORTS_END_PNG_PATH})
else()
  execute_process(
    COMMAND
      ln -s ${PROJECT_SOURCE_DIR}/resources/images/kylinsec-logo-fallbak.png
      ${RESOURCE_LOGO_PNG_PATH}
    COMMAND ln -s ${PROJECT_SOURCE_DIR}/resources/images/PDF/end-fallbak.png
            ${RESOURCE_REPORTS_END_PNG_PATH})
endif()

add_executable(
  ${TARGET_NAME}
  ${UI_H_FILES}
  ${UI_CPP_FILES}
  ${DAEMON_SRCS}
  ${BOX_MANAGER_SRCS}
  ${DEVICE_MANAGER_SRCS}
  ${KSS_DBUS_SRCS}
  ${BR_DBUS_SRCS}
  ${LOG_SRCS}
  ${TOOL_BOX_DBUS_SRCS}
  ${ACCOUNT_SRCS}
  ${BR_PROTOCOL_OUTPUT}
  ${QM_FILES}
  ${UI_FILES}
  "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS}
          ${X11_INCLUDE_DIRS} ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${X11_LIBRARIES}
          Qt5::DBus
          Qt5::Widgets
          Qt5::X11Extras
          Qt5::Network
          Qt5::PrintSupport
          lib-base
          lib-license
          qrencode
          ${XERCES_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
