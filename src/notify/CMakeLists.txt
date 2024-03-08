cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  PROPERTIES CLASSNAME ToolBoxDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  toolbox_dbus_proxy)

set(TARGET_NAME ks-ssr-notify)
set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR}/src/notify ${TS_FILES}
                       OPTIONS -I ${PROJECT_SOURCE_DIR})

file(GLOB_RECURSE NOTIFY_H_FILES ./*.h)
file(GLOB_RECURSE NOTIFY_CPP_FILES ./*.cpp)

add_executable(${TARGET_NAME} ${NOTIFY_H_FILES} ${NOTIFY_CPP_FILES}
                              ${TOOL_BOX_DBUS_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${CMAKE_BINARY_DIR})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::DBus lib-base)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
