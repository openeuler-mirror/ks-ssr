set(TARGET_NAME ui-tp)

file(GLOB_RECURSE TP_H_FILES ./*.h)
file(GLOB_RECURSE TP_CPP_FILES ./*.cpp)
file(GLOB_RECURSE TP_UI_FORMS_FILES ./*.ui)
set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
  PROPERTIES CLASSNAME KSSDbusProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  KSS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.KSS.xml
  kss_dbus_proxy)

qt5_wrap_ui(UI_FILES ${TP_UI_FORMS_FILES})

add_library(${TARGET_NAME} SHARED ${TP_H_FILES} ${TP_CPP_FILES}
                                  ${KSS_DBUS_SRCS} ${UI_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
