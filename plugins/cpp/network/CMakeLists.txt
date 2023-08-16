cmake_minimum_required(VERSION 3.2)

set (TARGET_NAME network)

file (GLOB_RECURSE NETWORK_H_FILES ./*.h)
file (GLOB_RECURSE NETWORK_CPP_FILES ./*.cpp)


add_library(${TARGET_NAME} SHARED
            ${NETWORK_H_FILES}
            ${NETWORK_CPP_FILES})


target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base
                      lib-config
                      lib-dbus)

execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/
                                          ${PROJECT_SOURCE_DIR}/plugins/cpp/network/ssr-plugin-network.xml.in 
                                          ${PROJECT_BINARY_DIR}/plugins/cpp/network/ssr-plugin-network.xml)

execute_process(COMMAND ${SED} -i -e "s/xml:lang/lang/g" ${PROJECT_BINARY_DIR}/plugins/cpp/network/ssr-plugin-network.xml)


install (FILES ${PROJECT_BINARY_DIR}/plugins/cpp/network/ssr-plugin-network.xml
        DESTINATION ${SSR_PLUGIN_ROOT_DIR}/)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_PLUGIN_ROOT_DIR}/cpp/)