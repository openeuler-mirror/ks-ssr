cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME report)

file(GLOB_RECURSE SOURCE_FILES *.h *.cpp *.ui)

add_library(${TARGET_NAME} STATIC ${SOURCE_FILES} res.qrc)

target_include_directories(
  ${TARGET_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
                        ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES} Qt5::Widgets
                                             Qt5::Network)
