cmake_minimum_required(VERSION 3.2)


set (TARGET_NAME audit)

file (GLOB_RECURSE AUDIT_H_FILES ./*.h)
file (GLOB_RECURSE AUDIT_CPP_FILES ./*.cpp)


add_library(${TARGET_NAME} SHARED
            ${AUDIT_H_FILES}
            ${AUDIT_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base
                      lib-config
                      lib-dbus)

GEN_AND_INSTALL_PLUGIN_XML(ssr-plugin-audit.xml.in)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_PLUGIN_ROOT_DIR}/cpp/)