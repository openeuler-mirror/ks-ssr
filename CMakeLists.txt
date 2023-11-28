cmake_minimum_required(VERSION 3.2)

project(ks-ssr VERSION 1.2)

add_compile_options(-fpic -Wall)

if(EXTRA_COMPILE_CHECK)
  add_compile_options(-Wextra -Wconversion -Wuninitialized)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# FIXME: 应该改成与版本无关，功能相关的名字，比如 ENABLE_HDMI
if(_345_GC_)
  add_definitions(-D_345_GC_)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# find_package(BR REQUIRED)
find_package(SSR REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(
  Qt5
  COMPONENTS Core
             Widgets
             DBus
             LinguistTools
             PrintSupport
             Network
             X11Extras
             Sql)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(XERCES REQUIRED xerces-c)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(LIBSYSTEMD REQUIRED libsystemd)
pkg_search_module(KS_LICENSE REQUIRED ks-license)
pkg_search_module(LIBCRYPTO REQUIRED libcrypto)
pkg_search_module(LIBAUDIT REQUIRED audit)
pkg_search_module(SQLCIPHER REQUIRED sqlcipher)
pkg_search_module(LIBSELINUX REQUIRED libselinux)
pkg_search_module(LIBNOTIFY REQUIRED libnotify)

string(REGEX MATCHALL "[0-9]+" LIBNOTIFY_VERSION_LIST ${LIBNOTIFY_VERSION})
list(GET LIBNOTIFY_VERSION_LIST 0 LIBNOTIFY_MAJOR)
list(GET LIBNOTIFY_VERSION_LIST 1 LIBNOTIFY_MINOR)

if(ENABLE_GTEST)
  pkg_search_module(GTEST REQUIRED gtest)
endif()

pkg_search_module(FMT QUIET fmt)
if(NOT FMT_FOUND)
  find_library(
    FMT_LIBRARIES
    NAMES fmt
    PATHS /usr/lib64/)
  if("${FMT_LIBRARIES}" STREQUAL "FMT_LIBRARIES-NOTFOUND")
    message(FATAL_ERROR "Not found fmt")
  endif()
endif()

# 优先使用 qt-5 的dbus工具，没有找到时寻找 qt-4 的版本。
find_program(QT_DBUS_XML2CPP qdbusxml2cpp-qt5 PATHS ${CMAKE_BINARY_DIR}/bin/)
if(NOT QT_DBUS_XML2CPP)
  find_program(DBUS_XML2CPP qdbusxml2cpp PATHS ${CMAKE_BINARY_DIR}/bin/)
  if(NOT QT_DBUS_XML2CPP)
    message(FATAL_ERROR "Not found qdbusxml2cpp-qt5 and qdbusxml2cpp")
  endif()
endif()

if(USE_PYTHON3)
  pkg_search_module(PYTHON QUIET python3-embed)
  if(NOT PYTHON_FOUND)
    pkg_search_module(PYTHON REQUIRED python3)
  endif()
else()
  pkg_search_module(PYTHON QUIET python2)
  if(NOT PYTHON_FOUND)
    # 兼容python2.6
    find_library(
      PYTHON_LIBRARIES
      NAMES python2.6
      PATHS /usr/lib64/)
    if("${PYTHON_LIBRARIES}" STREQUAL "PYTHON_LIBRARIES-NOTFOUND")
      message(FATAL_ERROR "Not found python2")
    else()
      set(PYTHON_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}/python2.6/")
    endif()
  endif()
endif()

find_program(XSDCXX NAMES "xsdcxx" REQUIRED)
find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)
find_program(SED NAMES "sed" REQUIRED)

set(SSR_DEVICE_UDEV_RULES_FILE "/etc/udev/rules.d/99-ssr-device.rules")
set(SSR_DEVICE_USB_DESC_FILE "/usr/share/hwdata/usb.ids")
configure_file(config.h.in ${PROJECT_BINARY_DIR}/config.h)

add_subdirectory(config)
add_subdirectory(lib)
add_subdirectory(resources)
add_subdirectory(src)
add_subdirectory(plugins/python)
add_subdirectory(data)

if(ENABLE_GTEST)
  add_subdirectory(test)
endif()
