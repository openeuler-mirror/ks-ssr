cmake_minimum_required(VERSION 3.0)

GEN_DBUS_CPP(SSR ssr com.kylinsec. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.Kiran.SSR.xml)

set (TARGET_NAME kiran-ssr-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${DAEMON_H_FILES}
               ${DAEMON_CPP_FILES}
               ${SSR_GENERATED_CPP_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
                           ${PROJECT_BINARY_DIR}/generated
                           ${JSONCPP_INCLUDE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
                      ${JSONCPP_LIBRARIES}
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_INSTALL_BINDIR}/)

