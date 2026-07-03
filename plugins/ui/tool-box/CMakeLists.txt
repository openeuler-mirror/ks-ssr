set(TARGET_NAME ui-tool-box)

file(GLOB_RECURSE TOOL_BOX_H_FILES ./*.h)
file(GLOB_RECURSE TOOL_BOX_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  PROPERTIES CLASSNAME ToolBoxDbusProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  toolbox_dbus_proxy)

qt5_wrap_ui(UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_library(${TARGET_NAME} SHARED ${TOOL_BOX_H_FILES} ${TOOL_BOX_CPP_FILES}
                                  ${TOOL_BOX_DBUS_SRCS} ${UI_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
