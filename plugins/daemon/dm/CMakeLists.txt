set(TARGET_NAME daemon-dm)

file(GLOB_RECURSE DM_H_FILES ./*.h)
file(GLOB_RECURSE DM_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/dbus.h
  KS::DM::DBus
  device_manager_adaptor
  DeviceManagerAdaptor)

add_library(${TARGET_NAME} SHARED ${DM_H_FILES} ${DM_CPP_FILES}
                                  ${DEVICE_MANAGER_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::DBus base dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
