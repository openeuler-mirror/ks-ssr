cmake_minimum_required(VERSION 2.8.12)

set (TARGET_NAME ks-ssr-config)

file(GLOB_RECURSE CONFIG_H_FILES ./*.h)
file(GLOB_RECURSE CONFIG_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${CONFIG_H_FILES}
               ${CONFIG_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_INSTALL_BINDIR}/)
              

