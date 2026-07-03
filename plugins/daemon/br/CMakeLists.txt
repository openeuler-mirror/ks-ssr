set(TARGET_NAME daemon-br)

# 检查依赖关系
pkg_search_module(XERCES REQUIRED xerces-c)
pkg_search_module(LIBAUDIT REQUIRED audit)

#
file(GLOB_RECURSE BR_H_FILES ./*.h)
file(GLOB_RECURSE BR_CPP_FILES ./*.cpp)

gen_protocol()

qt5_add_dbus_adaptor(
  BR_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.BR.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/dbus.h KS::BR::BRDBus br_adaptor BRAdaptor)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/daemon/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(COMMAND lupdate-qt5 -no-obsolete ${BR_H_FILES} ${BR_CPP_FILES}
                        -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${BR_H_FILES} ${BR_CPP_FILES} ${BR_DBUS_SRCS}
                                  ${BR_PROTOCOL_OUTPUT} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          ${KLOG_QT5_INCLUDE_DIRS}
          ${PYTHON_INCLUDE_DIRS}
          ${XERCES_INCLUDE_DIRS}
          ${LIBAUDIT_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES}
                         ${LIBAUDIT_LIBRARIES} Qt5::DBus base dbus)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_DAEMON_PLUGINDIR})
