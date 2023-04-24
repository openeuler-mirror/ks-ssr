cmake_minimum_required(VERSION 3.2)

project(ks-sc VERSION 1.0)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(SC REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets DBus LinguistTools X11Extras Sql)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(GTEST REQUIRED gtest)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(ECRYPTFS REQUIRED libecryptfs)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(GLIB2 REQUIRED glib-2.0)
pkg_search_module(X11 REQUIRED x11)
# pkg_search_module(KSS REQUIRED kss)
pkg_search_module(LIBSYSTEMD REQUIRED libsystemd)

set(SC_DEVICE_UDEV_RULES_FILE "/etc/udev/rules.d/99-ks-sc-device.rules")
configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(src)
