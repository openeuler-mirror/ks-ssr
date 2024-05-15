set(TARGET_NAME ui-dm)

#
file(GLOB_RECURSE DM_H_FILES ./*.h)
file(GLOB_RECURSE DM_CPP_FILES ./*.cpp)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
  PROPERTIES CLASSNAME DeviceManagerProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  DEVICE_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.DeviceManager.xml
  device_manager_proxy)

add_library(${TARGET_NAME} SHARED ${DM_H_FILES} ${DM_CPP_FILES}
                                  ${DEVICE_MANAGER_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
