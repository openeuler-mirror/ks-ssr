set(TARGET_NAME ui-remote)

file(GLOB_RECURSE REMOTE_H_FILES ./*.h)
file(GLOB_RECURSE REMOTE_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} SHARED ${REMOTE_H_FILES} ${REMOTE_CPP_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
                         ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(
  ${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} ${XERCES_LIBRARIES} Qt5::DBus
                         Qt5::Widgets base widgets)

install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_UI_PLUGINDIR})
