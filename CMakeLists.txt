cmake_minimum_required(VERSION 3.2)

project(ks-ssr VERSION 1.3)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

add_compile_options(-Wall)

option(EXTRA_COMPILE_CHECK "Enable extra compile check" OFF)
# option(ENABLE_ACCOUNT "Enable account" ON) option(ENABLE_LOG "Enable log
# support" ON)

# 由于禁用日志和账户模块后编译无法通过，所以只能启用。
set(ENABLE_ACCOUNT ON)
set(ENABLE_LOG ON)

option(ENABLE_BR "Enable base reinforcements module" ON)
option(ENABLE_HDMI_SUPPORT "Enable HDMI support" OFF)
option(ENABLE_KSS "Enable KSS module" OFF)
option(ENABLE_PRIVATE_BOX "Enable private box module" OFF)
option(ENABLE_TOOL_BOX "Enable tool box module" OFF)
option(ENABLE_DEVICE_MANAGER "Enable device manager module" OFF)
option(ENABLE_VULNERABILITY_MANAGER "Enable vulnerability manager module" OFF)
option(ENABLE_REMOTE_MANAGER "Enable remote manager module" OFF)
option(USE_PYTHON3 "Enable tool box module" OFF)

message(STATUS "EXTRA_COMPILE_CHECK: ${EXTRA_COMPILE_CHECK}")
message(STATUS "ENABLE_ACCOUNT: ${ENABLE_ACCOUNT}")
message(STATUS "ENABLE_HDMI_SUPPORT: ${ENABLE_HDMI_SUPPORT}")
message(STATUS "ENABLE_KSS: ${ENABLE_KSS}")
message(STATUS "ENABLE_LOG: ${ENABLE_LOG}")
message(STATUS "ENABLE_PRIVATE_BOX: ${ENABLE_PRIVATE_BOX}")
message(STATUS "ENABLE_TOOL_BOX: ${ENABLE_TOOL_BOX}")
message(STATUS "ENABLE_DEVICE_MANAGER: ${ENABLE_DEVICE_MANAGER}")
message(STATUS "ENABLE_VULNERABILITY_MANAGER: ${ENABLE_VULNERABILITY_MANAGER}")
message(STATUS "USE_PYTHON3: ${USE_PYTHON3}")

if(EXTRA_COMPILE_CHECK)
  add_compile_options(-Wextra -Wconversion -Wuninitialized)
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(ENABLE_HDMI_SUPPORT)
  add_definitions(-DENABLE_HDMI_SUPPORT)
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
             PrintSupport
             Network
             X11Extras
             Sql)

pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(LIBSYSTEMD REQUIRED libsystemd)
pkg_search_module(KS_LICENSE REQUIRED ks-license)
pkg_search_module(LIBCRYPTO REQUIRED libcrypto)
pkg_search_module(SQLCIPHER REQUIRED sqlcipher)

pkg_search_module(LIBNOTIFY REQUIRED libnotify)

string(REGEX MATCHALL "[0-9]+" LIBNOTIFY_VERSION_LIST ${LIBNOTIFY_VERSION})
list(GET LIBNOTIFY_VERSION_LIST 0 LIBNOTIFY_MAJOR)
list(GET LIBNOTIFY_VERSION_LIST 1 LIBNOTIFY_MINOR)

if(ENABLE_GTEST)
  pkg_search_module(GTEST REQUIRED gtest)
endif()

# 优先使用 qt-5 的dbus工具，没有找到时寻找 qt-4 的版本。
find_program(QT_DBUS_XML2CPP qdbusxml2cpp-qt5 PATHS ${CMAKE_BINARY_DIR}/bin/)
if(NOT QT_DBUS_XML2CPP)
  find_program(DBUS_XML2CPP qdbusxml2cpp PATHS ${CMAKE_BINARY_DIR}/bin/)
  if(NOT QT_DBUS_XML2CPP)
    message(FATAL_ERROR "Not found qdbusxml2cpp-qt5 and qdbusxml2cpp")
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
add_subdirectory(plugins)
add_subdirectory(data)

if(ENABLE_GTEST)
  add_subdirectory(test)
endif()
