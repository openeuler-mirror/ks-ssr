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
  SECURITY_BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.BoxManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/box/box-manager.h
  KS::BoxManager
  box_manager_adaptor
  BoxManagerAdaptor)

qt5_add_dbus_adaptor(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.KSS.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/kss/kss-dbus.h KS::KSSDbus kss_dbus_adaptor
  KSSDbusAdaptor)

qt5_add_dbus_adaptor(
  SECURITY_DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.DeviceManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/dm/device-dbus.h
  KS::DeviceDBus
  device_manager_adaptor
  DeviceManagerAdaptor)

qt5_add_dbus_adaptor(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SSR.BR.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/br/br-dbus.h KS::BRDaemon::BRDBus br_adaptor
  BRAdaptor)

set(TARGET_NAME ks-ssr-daemon)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} OPTIONS -I
                       ${PROJECT_SOURCE_DIR})

add_executable(
  ${TARGET_NAME}
  ${SRC_H_FILES}
  ${SRC_CPP_FILES}
  ${SECURITY_BOX_MANAGER_SRCS}
  ${TRUSTED_MANAGER_SRCS}
  ${FILES_PROTECTED_MANAGER_SRCS}
  ${SECURITY_DEVICE_MANAGER_SRCS}
  ${KSS_DBUS_SRCS}
  ${BR_DBUS_SRCS}
  ${BR_PROTOCOL_OUTPUT}
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
          ${PROJECT_SOURCE_DIR})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE Qt5::Core
          Qt5::DBus
          Qt5::Sql
          Qt5::X11Extras
          lib-base
          lib-license
          ${KLOG_QT_LIBRARIES}
          ${FMT_LIBRARIES}
          ${XERCES_LIBRARIES}
          ${PYTHON_LIBRARIES}
          ${LIBCRYPTO_LIBRARIES}
          ${LIBAUDIT_LIBRARIES}
          ${LIBSYSTEMD_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_BR_INSTALL_LIBEXECDIR})
