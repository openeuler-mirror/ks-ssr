cmake_minimum_required(VERSION 3.0)

GEN_DBUS_CPP(SSR ssr com.kylinsec. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.Kiran.SSR.xml)

set (TARGET_NAME kiran-ssr-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)

set(SSR_PROTOCOL_OUTPUT ${PROJECT_BINARY_DIR}/src/daemon/ssr-protocol.hxx
                        ${PROJECT_BINARY_DIR}/src/daemon/ssr-protocol.cxx)

add_custom_command(OUTPUT ${SSR_PROTOCOL_OUTPUT}
                   COMMAND ${XSDCXX} cxx-tree --std c++11 
                                              --namespace-map =Kiran::Protocol
                                              --type-naming ucc
                                              --generate-serialization
                                              --root-element-all
                                              ${PROJECT_SOURCE_DIR}/src/daemon/ssr-protocol.xsd
                   DEPENDS ${PROJECT_SOURCE_DIR}/src/daemon/ssr-protocol.xsd
                   COMMENT "generate the c++ file by ssr-protocol.xsd")


add_executable(${TARGET_NAME}
               ${DAEMON_H_FILES}
               ${DAEMON_CPP_FILES}
               ${SSR_GENERATED_CPP_FILES}
               ${SSR_PROTOCOL_OUTPUT})

target_include_directories(${TARGET_NAME} PRIVATE
                           ${PROJECT_BINARY_DIR}/generated
                           ${XERCES_INCLUDE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
                      ${XERCES_LIBRARIES}
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_INSTALL_BINDIR}/)

