cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

file(GLOB_RECURSE SRC_H_FILES ./*.h)
file(GLOB_RECURSE SRC_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  SECURITY_BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.BoxManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/box/box-manager.h
  KS::BoxManager
  box_manager_adaptor
  BoxManagerAdaptor)

qt5_add_dbus_adaptor(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.KSS.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/kss/kss-dbus.h KS::KSSDbus kss_dbus_adaptor
  KSSDbusAdaptor)

qt5_add_dbus_adaptor(
  SECURITY_DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.DeviceManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/device/device-dbus.h
  KS::DeviceDBus
  device_manager_adaptor
  DeviceManagerAdaptor)

set(TARGET_NAME ks-sc-daemon)

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
  ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE Qt5::Core Qt5::DBus Qt5::Sql Qt5::X11Extras lib-base
                         ${LIBSYSTEMD_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${KSC_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
