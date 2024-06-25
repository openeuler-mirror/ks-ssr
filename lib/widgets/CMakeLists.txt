set(TARGET_NAME widgets)

file(GLOB_RECURSE WIDGETS_H_FILES ./*.h)
file(GLOB_RECURSE WIDGETS_CPP_FILES ./*.cpp)
file(GLOB_RECURSE WIDGETS_UI_FILES ./*.ui)

set(ALL_FILES ${WIDGETS_H_FILES} ${WIDGETS_CPP_FILES} ${WIDGETS_UI_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                        ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES} Qt5::Widgets
                                             Qt5::Network Qt5::X11Extras)
