set(TARGET_NAME daemon-dm)

if(WITH_SYSTEMD_UDEV)
  file(GLOB DM_H_FILES ./*.h system-device/*.h system-device/systemd-udev/*.h)
  file(GLOB DM_CPP_FILES ./*.cpp system-device/*.cpp
       system-device/systemd-udev/*.cpp)
else()
  pkg_search_module(LIBUDEV REQUIRED libudev)

  file(GLOB DM_H_FILES ./*.h system-device/*.h system-device/udev/*.h)
  file(GLOB DM_CPP_FILES ./*.cpp system-device/*.cpp system-device/udev/*.cpp)
endif()

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

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::DBus base dbus
                                             ${LIBUDEV_LIBRARIES})

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
