cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0071)
  cmake_policy(SET CMP0071 NEW)
endif()

file(GLOB_RECURSE UI_H_FILES ./*.h)
file(GLOB_RECURSE UI_CPP_FILES ./*.cpp)

configure_file(config-ui.h.in ${PROJECT_BINARY_DIR}/config-ui.h)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.BoxManager.xml
  PROPERTIES CLASSNAME BoxManagerProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  BOX_MANAGER_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.BoxManager.xml
  box_manager_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.TrustedProtected.xml
  PROPERTIES CLASSNAME TPProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  TP_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.TrustedProtected.xml
  tp_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.FileProtected.xml
  PROPERTIES CLASSNAME FileProtectedProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  FILE_PROTECTED_SRCS
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.FileProtected.xml
  file_protected_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.DeviceManager.xml
  PROPERTIES CLASSNAME DeviceManagerProxy NO_NAMESPACE true)

qt5_add_dbus_interface(
  DEVICE_MANAGER_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.DeviceManager.xml
  device_manager_proxy)

qt5_wrap_ui(
  UI_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/window.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/license/license-activation.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/license/qrcode-dialog.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/common/pagination.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/box/box-page.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/box/create-box.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/box/modify-password.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/box/retrieve-password.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/box/input-password.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/fp/fp-page.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/device/device-page.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/device/sidebar-item.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/device/device-list.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/device/device-log.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/device/device-permission.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/tp/tp-kernel.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/tp/tp-execute.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/tp/tp-page.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/tp/table-delete-notify.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/setup/settings-page.ui
  ${CMAKE_CURRENT_SOURCE_DIR}/setup/trusted-settings.ui)

set(TARGET_NAME ks-sc-gui)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} OPTIONS -I
                       ${PROJECT_SOURCE_DIR})

add_executable(
  ${TARGET_NAME}
  ${UI_H_FILES}
  ${UI_CPP_FILES}
  ${BOX_MANAGER_SRCS}
  ${FILE_PROTECTED_SRCS}
  ${TP_SRCS}
  ${DEVICE_MANAGER_SRCS}
  ${QM_FILES}
  ${UI_FILES}
  "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS}
          ${X11_INCLUDE_DIRS}
          ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES} ${X11_LIBRARIES} Qt5::DBus
                         Qt5::Widgets Qt5::X11Extras lib-base qrencode)

install(FILES ${QM_FILES} DESTINATION ${KSC_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
