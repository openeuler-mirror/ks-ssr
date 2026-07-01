cmake_minimum_required(VERSION 3.0)

GEN_DBUS_CPP(SSE sse com.kylinsec. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.Kiran.SSE.xml)

set (TARGET_NAME kiran-sse-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${DAEMON_H_FILES}
               ${DAEMON_CPP_FILES}
               ${SSE_GENERATED_CPP_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
                           ${PROJECT_BINARY_DIR}/generated
                           ${JSONCPP_INCLUDE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
                      ${JSONCPP_LIBRARIES}
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSE_INSTALL_BINDIR}/)

