cmake_minimum_required(VERSION 3.2)

project(ks-ssr VERSION 1.2)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# FIXME: 应该改成与版本无关，功能相关的名字，比如 ENABLE_HDMI
if(_345_GC_)
  add_definitions(-D_345_GC_)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(SSR REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(
  Qt5
  COMPONENTS Core
             Widgets
             DBus
             LinguistTools
             Network
             X11Extras
             Sql)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(LIBSYSTEMD REQUIRED libsystemd)
pkg_search_module(KS_LICENSE REQUIRED ks-license)

set(SSR_DEVICE_UDEV_RULES_FILE "/etc/udev/rules.d/99-ssr-device.rules")
set(SSR_DEVICE_USB_DESC_FILE "/usr/share/hwdata/usb.ids")
configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(resources)
add_subdirectory(src)
