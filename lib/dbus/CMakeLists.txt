set(TARGET_NAME dbus)

file(GLOB_RECURSE DBUS_H_FILES ./*.h)
file(GLOB_RECURSE DBUS_CPP_FILES ./*.cpp)

set(ALL_FILES ${DBUS_H_FILES} ${DBUS_CPP_FILES})

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/lib/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(COMMAND lupdate-qt5 -no-obsolete ${ALL_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS}
         ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES}
                                             ${X11_LIBRARIES} Qt5::DBus)
