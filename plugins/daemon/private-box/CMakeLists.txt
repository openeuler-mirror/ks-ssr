set(TARGET_NAME daemon-private-box)

file(GLOB_RECURSE PRIVATE_BOX_H_FILES ./*.h)
file(GLOB_RECURSE PRIVATE_BOX_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  PRIVATE_BOX_MANAGER_SRCS
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.PrivateBox.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/box-manager.h
  KS::PrivateBox::BoxManager
  private_manager_adaptor
  BoxManagerAdaptor)

add_library(
  ${TARGET_NAME} SHARED ${PRIVATE_BOX_H_FILES} ${PRIVATE_BOX_CPP_FILES}
                        ${PRIVATE_BOX_MANAGER_SRCS})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Sql Qt5::DBus base dbus)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
