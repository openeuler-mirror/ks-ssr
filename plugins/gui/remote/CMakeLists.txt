set(TARGET_NAME gui-remote)

file(GLOB_RECURSE REMOTE_H_FILES ./*.h)
file(GLOB_RECURSE REMOTE_CPP_FILES ./*.cpp)
file(GLOB_RECURSE REMOTE_UI_FORMS_FILES ./*.ui)

qt5_wrap_ui(UI_FILES ${REMOTE_UI_FORMS_FILES})

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/gui/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(
  COMMAND lupdate-qt5 -no-obsolete ${REMOTE_H_FILES} ${REMOTE_CPP_FILES}
          ${REMOTE_UI_FORMS_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} SHARED ${REMOTE_H_FILES} ${REMOTE_CPP_FILES}
                                  ${UI_FILES} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
