cmake_minimum_required(VERSION 3.0)

file(GLOB_RECURSE SRC_H_FILES ./*.h)
file(GLOB_RECURSE SRC_CPP_FILES ./*.cpp)

qt_add_dbus_adaptor(
  SECURITY_BOX_MANAGER_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.BoxManager.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/box/box-manager.h KS::BoxManager
  box_manager_adaptor BoxManagerAdaptor)

qt_add_dbus_adaptor(
  TRUSTED_MANAGER_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.Trusted.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/trusted/trusted.h KS::Trusted
  trusted_adaptor TrustedAdaptor)

qt_add_dbus_adaptor(
  FILES_PROTECTED_MANAGER_SRCS ${CMAKE_SOURCE_DIR}/data/com.kylinsec.SC.FileProtected.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/protected/file-protected.h KS::FileProtected
  file_protected_adaptor FileProtectedAdaptor)

set(TARGET_NAME ks-sc-daemon)

set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(
  ${TARGET_NAME}
  ${SRC_H_FILES}
  ${SRC_CPP_FILES}
  ${SECURITY_BOX_MANAGER_SRCS}
  ${TRUSTED_MANAGER_SRCS}
  ${FILES_PROTECTED_MANAGER_SRCS}
  ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}
          ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include
          )

target_link_libraries(
  ${TARGET_NAME}
  PRIVATE Qt5::Core
          Qt5::DBus
          Qt5::Sql
          lib-base)

install(FILES ${QM_FILES} DESTINATION ${SC_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_FULL_BINDIR})
