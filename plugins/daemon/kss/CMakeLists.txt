set(TARGET_NAME daemon-kss)

file(GLOB_RECURSE KSS_H_FILES ./*.h)
file(GLOB_RECURSE KSS_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/dbus.h KS::KSS::DBus kss_dbus_adaptor
  KSSDbusAdaptor)

add_library(${TARGET_NAME} SHARED ${KSS_H_FILES} ${KSS_CPP_FILES}
                                  ${KSS_DBUS_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::DBus base dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
