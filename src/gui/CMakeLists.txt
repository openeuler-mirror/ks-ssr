file(
  GLOB
  UI_H_FILES
  ${PROJECT_SOURCE_DIR}/include/page.h
  ${PROJECT_SOURCE_DIR}/include/ui-plugin-i.h
  accounts/*.h
  common/*.h
  common/*/*.h
  license/*.h
  log/*.h
  ./*.h)

file(
  GLOB
  UI_CPP_FILES
  accounts/*.cpp
  common/*.cpp
  common/*/*.cpp
  license/*.cpp
  log/*.cpp
  ./*.cpp)

file(GLOB_RECURSE UI_FORMS_FILES ./*.ui)

configure_file(config-ui.h.in ${PROJECT_BINARY_DIR}/config-ui.h)

set_source_files_properties(${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml
                            PROPERTIES CLASSNAME DaemonProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  DAEMON_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml daemon_proxy)

set_source_files_properties(
  ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
  PROPERTIES CLASSNAME AccountProxy NO_NAMESPACE true)
qt5_add_dbus_interface(
  ACCOUNT_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
  account_proxy)

qt5_wrap_ui(UI_FILES ${UI_FORMS_FILES})

set(TARGET_NAME ${PROJECT_NAME}-gui)

# 更新ts文件
set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/gui/${TARGET_NAME}.zh_CN.ts")
execute_process(
  COMMAND lupdate-qt5 -no-obsolete ${PROJECT_SOURCE_DIR}/lib ${UI_H_FILES}
          ${UI_CPP_FILES} ${UI_FORMS_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

set(RESOURCE_LOGO_PNG_PATH
    "${PROJECT_SOURCE_DIR}/resources/images/kylinsec-logo.png")
set(RESOURCE_REPORTS_END_PNG_PATH
    "${PROJECT_SOURCE_DIR}/resources/images/PDF/end.png")

# 判断private目录是否拉取，替换logo图片
if(EXISTS "${PROJECT_SOURCE_DIR}/private/check-project-exist")
  execute_process(
    COMMAND
      ln -s ${PROJECT_SOURCE_DIR}/private/resources/images/kylinsec-logo.png
      ${RESOURCE_LOGO_PNG_PATH}
    COMMAND ln -s ${PROJECT_SOURCE_DIR}/private/resources/images/PDF/end.png
            ${RESOURCE_REPORTS_END_PNG_PATH})
else()
  execute_process(
    COMMAND
      ln -s ${PROJECT_SOURCE_DIR}/resources/images/kylinsec-logo-fallbak.png
      ${RESOURCE_LOGO_PNG_PATH}
    COMMAND ln -s ${PROJECT_SOURCE_DIR}/resources/images/PDF/end-fallbak.png
            ${RESOURCE_REPORTS_END_PNG_PATH})
endif()

add_executable(
  ${TARGET_NAME}
  ${UI_H_FILES}
  ${UI_CPP_FILES}
  ${DAEMON_SRCS}
  ${ACCOUNT_SRCS}
  ${QM_FILES}
  ${UI_FILES}
  "${PROJECT_SOURCE_DIR}/resources/resources.qrc")

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS}
          ${X11_INCLUDE_DIRS} ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE ${KLOG_QT5_LIBRARIES}
          ${X11_LIBRARIES}
          Qt5::DBus
          Qt5::Widgets
          Qt5::PrintSupport
          Qt5::X11Extras
          base
          dbus
          widgets
          qrencode)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_LIBEXECDIR})
install(
  PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/ks-ssr-gui.sh"
  RENAME ${TARGET_NAME}
  DESTINATION ${SSR_INSTALL_BINDIR})

