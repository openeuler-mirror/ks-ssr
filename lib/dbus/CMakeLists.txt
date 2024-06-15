set(TARGET_NAME dbus)

file(GLOB_RECURSE DBUS_H_FILES ./*.h)
file(GLOB_RECURSE DBUS_CPP_FILES ./*.cpp)

set(ALL_FILES ${DBUS_H_FILES} ${DBUS_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR} ${KLOG_QT5_INCLUDE_DIRS}
         ${KS_LICENSE_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PRIVATE ${KLOG_QT5_LIBRARIES}
                                             ${X11_LIBRARIES} Qt5::DBus)
