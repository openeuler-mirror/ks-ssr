set(TARGET_NAME daemon-tool-box)

pkg_search_module(LIBSELINUX REQUIRED libselinux)

file(GLOB_RECURSE TOOL_BOX_H_FILES ./*.h)
file(GLOB_RECURSE TOOL_BOX_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/manager.h KS::ToolBox::Manager tool_box_adaptor
  ToolBoxAdaptor)

add_library(${TARGET_NAME} SHARED ${TOOL_BOX_H_FILES} ${TOOL_BOX_CPP_FILES}
                                  ${TOOL_BOX_DBUS_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${LIBSELINUX_LIBRARIES} Qt5::DBus
                                             base dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
