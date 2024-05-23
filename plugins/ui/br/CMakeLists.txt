set(TARGET_NAME ui-br)

# 检查依赖关系
pkg_search_module(XERCES REQUIRED xerces-c)

#
file(GLOB_RECURSE BR_H_FILES ./*.h)
file(GLOB_RECURSE BR_CPP_FILES ./*.cpp)

gen_protocol()

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  PROPERTIES CLASSNAME BRDbusProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  br_dbus_proxy)

qt5_wrap_ui(UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_library(${TARGET_NAME} SHARED ${BR_H_FILES} ${BR_CPP_FILES} ${BR_DBUS_SRCS}
                                  ${UI_FILES} ${BR_PROTOCOL_OUTPUT})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include ${PYTHON_INCLUDE_DIRS}
          ${XERCES_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${PYTHON_LIBRARIES}
          ${XERCES_LIBRARIES}
          Qt5::DBus
          Qt5::Widgets
          Qt5::Network
          base
          widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
