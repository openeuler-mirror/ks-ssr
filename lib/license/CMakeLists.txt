cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME lib-license)

file(GLOB_RECURSE LICENSE_H_FILES ./*.h)
file(GLOB_RECURSE LICENSE_CPP_FILES ./*.cpp)

set(ALL_FILES ${LICENSE_H_FILES} ${LICENSE_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS}
         ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES}
                                             ${X11_LIBRARIES} Qt5::DBus)
