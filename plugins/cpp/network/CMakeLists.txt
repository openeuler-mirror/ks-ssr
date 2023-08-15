cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME network)

file (GLOB_RECURSE NETWORK_H_FILES ./*.h)
file (GLOB_RECURSE NETWORK_CPP_FILES ./*.cpp)


add_library(${TARGET_NAME} SHARED
            ${NETWORK_H_FILES}
            ${NETWORK_CPP_FILES})


target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSE_PLUGIN_ROOT_DIR}/cpp/)