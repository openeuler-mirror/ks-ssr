cmake_minimum_required(VERSION 3.0)

set (TARGET_NAME kiran-sse-tool)

file(GLOB_RECURSE TOOL_H_FILES ./*.h)
file(GLOB_RECURSE TOOL_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${TOOL_H_FILES}
               ${TOOL_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)

