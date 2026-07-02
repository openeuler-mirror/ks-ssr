cmake_minimum_required(VERSION 3.0)

GEN_DBUS_CPP(SSR ssr com.kylinsec. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.Kiran.SSR.xml)
GEN_PROTOCOL()

set (TARGET_NAME kiran-ssr-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)

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

