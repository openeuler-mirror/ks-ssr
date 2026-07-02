
cmake_minimum_required(VERSION 3.2)

project(kiran-ssr-manager VERSION 1.0.0)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(SSR REQUIRED)

find_package(PkgConfig REQUIRED)

pkg_search_module(SYSTEMD REQUIRED systemd)
pkg_search_module(GIOMM REQUIRED giomm-2.4)
pkg_search_module(GTHREAD REQUIRED gthread-2.0)
pkg_search_module(GMODULE REQUIRED gmodule-2.0)
pkg_search_module(GIOUNIX REQUIRED gio-unix-2.0)
pkg_search_module(KLOG REQUIRED klog-gtk3)
pkg_search_module(JSONCPP REQUIRED jsoncpp)
pkg_search_module(GTEST REQUIRED gtest)
pkg_search_module(FMT REQUIRED fmt)
pkg_search_module(CRYPTOPP REQUIRED cryptopp)
pkg_search_module(XERCES REQUIRED xerces-c)


find_program(GDBUS-CODEGEN-GLIBMM-FOUND gdbus-codegen-glibmm3 PATHS ${CMAKE_BINARY_DIR}/bin/)
find_program(XSDCXX NAMES "xsdcxx" REQUIRED)
find_program(INTLTOOL-MERGE NAMES "intltool-merge" REQUIRED)
find_program(SED NAMES "sed" REQUIRED)


SET_COMMON_COMPILER_FLAGS()


if (NOT GDBUS-CODEGEN-GLIBMM-FOUND)
    message(FATAL_ERROR "not found gdbus-codegen-glibmm3.")
else()
    SET (GDBUS_CODEGEN ${GDBUS-CODEGEN-GLIBMM-FOUND})
endif()

configure_file(ssr-config.h.in ${PROJECT_BINARY_DIR}/ssr-config.h)

add_subdirectory(po)
add_subdirectory(data)
add_subdirectory(lib)
add_subdirectory(src/daemon)
add_subdirectory(src/tool)
add_subdirectory(plugins/cpp)
add_subdirectory(test)


