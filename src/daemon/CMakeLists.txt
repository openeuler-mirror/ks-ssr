cmake_minimum_required(VERSION 2.8.12)

GEN_DBUS_CPP(SSR ssr com. ${CMAKE_SOURCE_DIR}/src/daemon/com.kylinsec.SSR.xml)
GEN_DBUS_CPP(LICENSE_MANAGER license_manager com.kylinsec. ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.LicenseManager.xml)
GEN_DBUS_CPP(LICENSE_OBJECT license_object com.kylinsec. ${CMAKE_CURRENT_SOURCE_DIR}/com.kylinsec.Kiran.LicenseObject.xml)
GEN_PROTOCOL()

set (TARGET_NAME ks-ssr-daemon)

file(GLOB_RECURSE DAEMON_H_FILES ./*.h)
file(GLOB_RECURSE DAEMON_CPP_FILES ./*.cpp)

add_executable(${TARGET_NAME}
               ${DAEMON_H_FILES}
               ${DAEMON_CPP_FILES}
               ${SSR_GENERATED_CPP_FILES}
               ${LICENSE_MANAGER_GENERATED_CPP_FILES}
               ${LICENSE_OBJECT_GENERATED_CPP_FILES}
               ${SSR_PROTOCOL_OUTPUT})


target_include_directories(${TARGET_NAME} PRIVATE
                           ${PROJECT_BINARY_DIR}/generated
                           ${XERCES_INCLUDE_DIRS}
                           ${PYTHON_INCLUDE_DIRS}
                           ${GUDEV_INCLUDE_DIRS}
                           ${LIBCRYPTO_INCLUDE_DIRS}
                           ${KYLIN_LICENSE_INCLUDE_DIRS}
                           ${DBUSCXX_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PUBLIC
                      ${XERCES_LIBRARIES}
                      ${PYTHON_LIBRARIES}
                      ${GUDEV_LIBRARIES}
                      ${LIBCRYPTO_LIBRARIES}
                      ${DBUSCXX_LIBRARIES}
                      lib-base)

install(TARGETS ${TARGET_NAME}
        DESTINATION ${SSR_INSTALL_BINDIR}/)

