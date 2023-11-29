cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

# gen_dbus_cpp(BR br com.
# ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.SSR.BR.xml)
gen_protocol()

# 打开全局moc
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SRC_H_FILES ./*.h)
file(GLOB_RECURSE SRC_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  PRIVATE_BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.PrivateBox.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/private-box/box-manager.h
  KS::PrivateBox::BoxManager
  private_manager_adaptor
  BoxManagerAdaptor)

qt5_add_dbus_adaptor(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.KSS.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/kss/dbus.h KS::KSS::DBus kss_dbus_adaptor
  KSSDbusAdaptor)

qt5_add_dbus_adaptor(
  SECURITY_DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.DeviceManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/dm/dbus.h
  KS::DM::DBus
  device_manager_adaptor
  DeviceManagerAdaptor)

qt5_add_dbus_adaptor(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.BR.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/br/br-dbus.h KS::BRDaemon::BRDBus br_adaptor
  BRAdaptor)

qt5_add_dbus_adaptor(
  LOG_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.Log.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/log/manager.h KS::Log::Manager log_adaptor
  LogAdaptor)

qt5_add_dbus_adaptor(
  ACCOUNT_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.Account.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/account/manager.h KS::Account::Manager
  account_adaptor AccountAdaptor)

qt5_add_dbus_adaptor(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.ToolBox.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/tool-box/manager.h KS::ToolBox::Manager
  tool_box_adaptor ToolBoxAdaptor)

set(TARGET_NAME ks-ssr-daemon)

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

add_executable(
  ${TARGET_NAME}
  ${SRC_H_FILES}
  ${SRC_CPP_FILES}
  ${PRIVATE_BOX_MANAGER_SRCS}
  ${TRUSTED_MANAGER_SRCS}
  ${FILES_PROTECTED_MANAGER_SRCS}
  ${SECURITY_DEVICE_MANAGER_SRCS}
  ${KSS_DBUS_SRCS}
  ${BR_DBUS_SRCS}
  ${LOG_DBUS_SRCS}
  ${ACCOUNT_DBUS_SRCS}
  ${BR_PROTOCOL_OUTPUT}
  ${TOOL_BOX_DBUS_SRCS}
  ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${PROJECT_BINARY_DIR}/generated
          ${XERCES_INCLUDE_DIRS}
          ${PYTHON_INCLUDE_DIRS}
          ${LIBCRYPTO_INCLUDE_DIRS}
          ${LIBAUDIT_INCLUDE_DIRS}
          ${Qt5Network_INCLUDE_DIRS}
          ${PROJECT_SOURCE_DIR})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE Qt5::Core
          Qt5::DBus
          Qt5::Sql
          Qt5::X11Extras
          Qt5::Network
          lib-base
          lib-license
          ${KLOG_QT_LIBRARIES}
          ${FMT_LIBRARIES}
          ${XERCES_LIBRARIES}
          ${PYTHON_LIBRARIES}
          ${LIBCRYPTO_LIBRARIES}
          ${LIBAUDIT_LIBRARIES}
          ${LIBSYSTEMD_LIBRARIES}
          ${LIBSELINUX_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_BR_INSTALL_LIBEXECDIR})
