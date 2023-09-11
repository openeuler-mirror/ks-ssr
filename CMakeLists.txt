
cmake_minimum_required(VERSION 2.8.12)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
    project(ks-ssr-manager VERSION 1.1)
else()
    project(ks-ssr-manager)
    set (PROJECT_VERSION 1.1)
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(SSR REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
pkg_search_module(GIOUNIX REQUIRED gio-unix-2.0)
pkg_search_module(KLOG REQUIRED klog-gtk3)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(XERCES REQUIRED xerces-c)
pkg_search_module(GUDEV REQUIRED gudev-1.0)
pkg_search_module(LIBCRYPTO REQUIRED libcrypto)
pkg_search_module(KYLIN_LICENSE REQUIRED kylin-license)

if(ENABLE_GTEST)
    pkg_search_module(GTEST REQUIRED gtest)
endif()

if(USE_SYSTEMD)
    pkg_search_module(SYSTEMD REQUIRED systemd)
endif()

pkg_search_module(FMT QUIET fmt)
if(NOT FMT_FOUND)
    find_library(FMT_LIBRARIES NAMES fmt
                 PATHS /usr/lib64/)
    if("${FMT_LIBRARIES}" STREQUAL "FMT_LIBRARIES-NOTFOUND")
        message (FATAL_ERROR "Not found fmt")
    endif()
endif()

pkg_search_module(DBUSCXX QUIET dbus-c++-glib-1)
if(NOT DBUSCXX_FOUND)
    pkg_search_module(DBUSCXX REQUIRED dbus-c++-1)
endif()

find_program(DBUSXX_XML2CPP dbusxx-xml2cpp PATHS ${CMAKE_BINARY_DIR}/bin/)
if(NOT DBUSXX_XML2CPP)
    message(FATAL_ERROR "Not found dbusxx-xml2cpp")
endif()


if(USE_PYTHON3)
    pkg_search_module(PYTHON QUIET python3-embed)
    if (NOT PYTHON_FOUND)
        pkg_search_module(PYTHON REQUIRED python3)
    endif()
else()
    pkg_search_module(PYTHON QUIET python2)
    if (NOT PYTHON_FOUND)
        # 兼容python2.6
        find_library(PYTHON_LIBRARIES NAMES python2.6 PATHS /usr/lib64/)
        if("${PYTHON_LIBRARIES}" STREQUAL "PYTHON_LIBRARIES-NOTFOUND")
            message (FATAL_ERROR "Not found python2")
        else()
            set(PYTHON_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}/python2.6/")
        endif()
    endif()
endif()


find_program(XSDCXX NAMES "xsdcxx" REQUIRED)
find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)
find_program(SED NAMES "sed" REQUIRED)



configure_file(ssr-config.h.in ${PROJECT_BINARY_DIR}/ssr-config.h)

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(config)
add_subdirectory(src/daemon)
add_subdirectory(src/tool)
#add_subdirectory(plugins/cpp)
add_subdirectory(plugins/python)

if(ENABLE_GTEST)
    add_subdirectory(test)
endif()


