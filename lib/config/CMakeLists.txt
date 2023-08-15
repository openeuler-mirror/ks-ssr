cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME lib-config)

file(GLOB_RECURSE CONFIG_H_FILES ./*.h)
file(GLOB_RECURSE CONFIG_CPP_FILES ./*.cpp)

set (ALL_FILES ${CONFIG_H_FILES} ${CONFIG_CPP_FILES})

add_library(${TARGET_NAME} ${ALL_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)