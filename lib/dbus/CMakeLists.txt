cmake_minimum_required(VERSION 2.8.12)

set (TARGET_NAME lib-dbus)

file(GLOB_RECURSE DBUS_H_FILES ./*.h)
file(GLOB_RECURSE DBUS_CPP_FILES ./*.cpp)

set (ALL_FILES ${DBUS_H_FILES} ${DBUS_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)