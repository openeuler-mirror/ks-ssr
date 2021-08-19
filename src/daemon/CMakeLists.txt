cmake_minimum_required(VERSION 3.0)

GEN_DBUS_CPP(SSR ssr com.kylinsec. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.Kiran.SSR.xml)

set (TARGET_NAME kiran-ssr-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)

set(PLUGIN_CONFIG_OUTPUT ${PROJECT_BINARY_DIR}/src/daemon/ssr-plugin-config.hxx
                         ${PROJECT_BINARY_DIR}/src/daemon/ssr-plugin-config.cxx)

set(RS_CONFIG_OUTPUT ${PROJECT_BINARY_DIR}/src/daemon/ssr-rs-config.hxx
                     ${PROJECT_BINARY_DIR}/src/daemon/ssr-rs-config.cxx)

add_custom_command(OUTPUT ${PLUGIN_CONFIG_OUTPUT}
                   COMMAND ${XSDCXX} cxx-tree --std c++11 
                                              --namespace-map =Kiran::Plugin
                                              --type-naming ucc
                                              ${PROJECT_SOURCE_DIR}/src/daemon/ssr-plugin-config.xsd
                   COMMENT "generate the c++ file by ssr-plugin-config.xsd")

add_custom_command(OUTPUT ${RS_CONFIG_OUTPUT}
                   COMMAND ${XSDCXX} cxx-tree --std c++11 
                                              --namespace-map =Kiran::RS
                                              --type-naming ucc
                                              --generate-serialization
                                              --root-element ssr_rs
                                              --root-element ssr_ra
                                              ${PROJECT_SOURCE_DIR}/src/daemon/ssr-rs-config.xsd
                   COMMENT "generate the c++ file by ssr-rs-config.xsd")


add_executable(${TARGET_NAME}
               ${DAEMON_H_FILES}
               ${DAEMON_CPP_FILES}
               ${SSR_GENERATED_CPP_FILES}
               ${PLUGIN_CONFIG_OUTPUT}
               ${RS_CONFIG_OUTPUT})

target_include_directories(${TARGET_NAME} PRIVATE
                           ${PROJECT_BINARY_DIR}/generated
                           ${XERCES_INCLUDE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
                      ${XERCES_LIBRARIES}
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_INSTALL_BINDIR}/)

