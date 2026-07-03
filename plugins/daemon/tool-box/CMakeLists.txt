set(TARGET_NAME daemon-tool-box)

pkg_search_module(LIBSELINUX REQUIRED libselinux)

file(GLOB_RECURSE TOOL_BOX_H_FILES ./*.h)
file(GLOB_RECURSE TOOL_BOX_CPP_FILES ./*.cpp)

qt5_add_dbus_adaptor(
  TOOL_BOX_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.ToolBox.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/manager.h KS::ToolBox::Manager tool_box_adaptor
  ToolBoxAdaptor)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/daemon/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(COMMAND lupdate-qt5 -no-obsolete ${TOOL_BOX_H_FILES}
                        ${TOOL_BOX_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${TOOL_BOX_H_FILES} ${TOOL_BOX_CPP_FILES}
                                  ${TOOL_BOX_DBUS_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${LIBSELINUX_LIBRARIES} Qt5::DBus
                                             base dbus)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
