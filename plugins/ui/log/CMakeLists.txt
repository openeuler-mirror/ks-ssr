set(TARGET_NAME ui-log)

file(GLOB_RECURSE LOG_H_FILES ./*.h)
file(GLOB_RECURSE LOG_CPP_FILES ./*.cpp)
file(GLOB_RECURSE LOG_UI_FORMS_FILES ./*.ui)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml
  PROPERTIES CLASSNAME LogProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  LOG_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml log_proxy)

qt5_wrap_ui(UI_FILES ${FP_UI_FORMS_FILES})

add_library(${TARGET_NAME} SHARED ${LOG_H_FILES} ${LOG_CPP_FILES} ${LOG_SRCS}
                                  ${LOG_UI_FORMS_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
