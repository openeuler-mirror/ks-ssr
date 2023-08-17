cmake_minimum_required(VERSION 3.2)


set (TARGET_NAME config)

file (GLOB_RECURSE CONFIG_H_FILES ./*.h)
file (GLOB_RECURSE CONFIG_CPP_FILES ./*.cpp)


add_library(${TARGET_NAME} SHARED
            ${CONFIG_H_FILES}
            ${CONFIG_CPP_FILES})

#target_include_directories(${TARGET_NAME} PRIVATE
#                           )

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_PLUGIN_ROOT_DIR}/cpp/)