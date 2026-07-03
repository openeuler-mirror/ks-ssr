set(TARGET_NAME ui-private-box)

file(GLOB_RECURSE PRIVATE_BOX_H_FILES ./*.h)
file(GLOB_RECURSE PRIVATE_BOX_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
  PROPERTIES CLASSNAME BoxManagerProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
  box_manager_proxy)

add_library(${TARGET_NAME} SHARED ${PRIVATE_BOX_H_FILES}
                                  ${PRIVATE_BOX_CPP_FILES} ${BOX_MANAGER_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
