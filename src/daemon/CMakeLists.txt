cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

# gen_dbus_cpp(BR br com.
# ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.SSR.BR.xml)
gen_protocol()

# 打开全局moc
set(CMAKE_AUTOMOC ON)

file(GLOB_RECURSE SRC_H_FILES ./br/*.h ./common/*.h ./daemon.h)
file(GLOB_RECURSE SRC_CPP_FILES ./br/*.cpp ./common/*.cpp ./daemon.cpp
     ./main.cpp)

if(ENABLE_ACCOUNT)
  file(GLOB_RECURSE SRC_ACCOUNT_H_FILES ./account/*.h)
  list(APPEND SRC_H_FILES ${SRC_ACCOUNT_H_FILES})
  file(GLOB_RECURSE SRC_ACCOUNT_CPP_FILES ./account/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_ACCOUNT_CPP_FILES})

  qt5_add_dbus_adaptor(
    ACCOUNT_DBUS_SRCS
    ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/account/manager.h
    KS::Account::Manager
    account_adaptor
    AccountAdaptor)

  add_compile_definitions(ENABLE_ACCOUNT)
endif()

if(ENABLE_KSS)
  file(GLOB_RECURSE SRC_KSS_H_FILES ./kss/*.h)
  list(APPEND SRC_H_FILES ${SRC_KSS_H_FILES})
  file(GLOB_RECURSE SRC_KSS_CPP_FILES ./kss/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_KSS_CPP_FILES})

  qt5_add_dbus_adaptor(
    KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/kss/dbus.h KS::KSS::DBus kss_dbus_adaptor
    KSSDbusAdaptor)

  add_compile_definitions(ENABLE_KSS)
endif()

if(ENABLE_LOG)
  file(GLOB_RECURSE SRC_LOG_H_FILES ./log/*.h)
  list(APPEND SRC_H_FILES ${SRC_LOG_H_FILES})
  file(GLOB_RECURSE SRC_LOG_CPP_FILES ./log/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_LOG_CPP_FILES})

  qt5_add_dbus_adaptor(
    LOG_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/log/manager.h KS::Log::Manager log_adaptor
    LogAdaptor)

  add_compile_definitions(ENABLE_LOG)
endif()

if(ENABLE_PRIVATE_BOX)
  file(GLOB_RECURSE SRC_PRIVATE_BOX_H_FILES ./private-box/*.h)
  list(APPEND SRC_H_FILES ${SRC_PRIVATE_BOX_H_FILES})
  file(GLOB_RECURSE SRC_PRIVATE_BOX_CPP_FILES ./private-box/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_PRIVATE_BOX_CPP_FILES})

  qt5_add_dbus_adaptor(
    PRIVATE_BOX_MANAGER_SRCS
    ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/private-box/box-manager.h
    KS::PrivateBox::BoxManager
    private_manager_adaptor
    BoxManagerAdaptor)

  add_compile_definitions(ENABLE_PRIVATE_BOX)
endif()

if(ENABLE_TOOL_BOX)
  file(GLOB_RECURSE SRC_TOOL_BOX_H_FILES ./tool-box/*.h)
  list(APPEND SRC_H_FILES ${SRC_TOOL_BOX_H_FILES})
  file(GLOB_RECURSE SRC_TOOL_BOX_CPP_FILES ./tool-box/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_TOOL_BOX_CPP_FILES})

  qt5_add_dbus_adaptor(
    TOOL_BOX_DBUS_SRCS
    ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/tool-box/manager.h
    KS::ToolBox::Manager
    tool_box_adaptor
    ToolBoxAdaptor)

  add_compile_definitions(ENABLE_TOOL_BOX)
endif()

if(ENABLE_DEVICE_MANAGER)
  file(GLOB_RECURSE SRC_DEVICE_MANAGER_H_FILES ./dm/*.h)
  list(APPEND SRC_H_FILES ${SRC_DEVICE_MANAGER_H_FILES})
  file(GLOB_RECURSE SRC_DEVICE_MANAGER_CPP_FILES ./dm/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_DEVICE_MANAGER_CPP_FILES})

  qt5_add_dbus_adaptor(
    DEVICE_MANAGER_SRCS
    ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/dm/dbus.h
    KS::DM::DBus
    device_manager_adaptor
    DeviceManagerAdaptor)

  add_compile_definitions(ENABLE_DEVICE_MANAGER)
endif()

if(ENABLE_VULNERABILITY_MANAGER)
  file(GLOB_RECURSE SRC_VULNERABILITY_MANAGER_H_FILES ./vulnerability/*.h)
  list(APPEND SRC_H_FILES ${SRC_VULNERABILITY_MANAGER_H_FILES})
  file(GLOB_RECURSE SRC_VULNERABILITY_MANAGER_CPP_FILES ./vulnerability/*.cpp)
  list(APPEND SRC_CPP_FILES ${SRC_VULNERABILITY_MANAGER_CPP_FILES})

  qt5_add_dbus_adaptor(
    VULNERABILITY_MANAGER_SRCS
    ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Vulnerability.xml
    ${CMAKE_SOURCE_DIR}/src/daemon/vulnerability/manager.h
    KS::VulnerabilityManager::Manager
    vulnerability_manager_adaptor
    VulnerabilityManagerAdaptor)

  add_compile_definitions(ENABLE_VULNERABILITY_MANAGER)
endif()

qt5_add_dbus_adaptor(
  DAEMON_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/daemon.h KS::Daemon daemon_adaptor
  DaemonAdaptor)

qt5_add_dbus_adaptor(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/br/br-dbus.h KS::BRDaemon::BRDBus br_adaptor
  BRAdaptor)

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
  ${DAEMON_SRCS}
  ${PRIVATE_BOX_MANAGER_SRCS}
  ${TRUSTED_MANAGER_SRCS}
  ${FILES_PROTECTED_MANAGER_SRCS}
  ${DEVICE_MANAGER_SRCS}
  ${KSS_DBUS_SRCS}
  ${BR_DBUS_SRCS}
  ${LOG_DBUS_SRCS}
  ${ACCOUNT_DBUS_SRCS}
  ${BR_PROTOCOL_OUTPUT}
  ${TOOL_BOX_DBUS_SRCS}
  ${VULNERABILITY_MANAGER_SRCS}
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
          ${LIBSELINUX_LIBRARIES}
          auparse)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_BR_INSTALL_LIBEXECDIR})
