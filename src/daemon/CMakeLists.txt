set(TARGET_NAME ks-ssr-daemon)

if(USE_PYTHON3)
  pkg_search_module(PYTHON QUIET python3-embed)
  if(NOT PYTHON_FOUND)
    pkg_search_module(PYTHON REQUIRED python3)
  endif()
else()
  pkg_search_module(PYTHON QUIET python2)
  if(NOT PYTHON_FOUND)
    # 兼容python2.6
    find_library(
      PYTHON_LIBRARIES
      NAMES python2.6
      PATHS /usr/lib64/)
    if("${PYTHON_LIBRARIES}" STREQUAL "PYTHON_LIBRARIES-NOTFOUND")
      message(FATAL_ERROR "Not found python2")
    else()
      set(PYTHON_INCLUDE_DIRS "${CMAKE_INSTALL_FULL_INCLUDEDIR}/python2.6/")
    endif()
  endif()
endif()

file(GLOB SRC_H_FILES accounts/*.h log/*.h ./*.h)
file(GLOB SRC_CPP_FILES accounts/*.cpp log/*.cpp ./*.cpp)

qt5_add_dbus_adaptor(
  DAEMON_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/daemon.h KS::Daemon daemon_adaptor
  DaemonAdaptor)

qt5_add_dbus_adaptor(
  ACCOUNTS_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Account.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/accounts/manager.h KS::Accounts::Manager
  accounts_adaptor AccountsAdaptor)

qt5_add_dbus_adaptor(
  LOG_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/log/manager.h KS::Log::Manager log_adaptor
  LogAdaptor)

# TODO: 插件的翻译后面要放到插件里面去
set(TS_FILES "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.zh_CN.ts")
qt5_create_translation(
  QM_FILES
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/data/br-system-rs.xml.in
  ${CMAKE_SOURCE_DIR}/data/br-categories.ini.in
  ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-audit.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-config.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-external.xml.in
  ${CMAKE_SOURCE_DIR}/plugins/reinforcements/python/br-plugin-network.xml.in
  ${TS_FILES}
  OPTIONS
  -I
  ${PROJECT_SOURCE_DIR})

add_executable(
  ${TARGET_NAME} ${SRC_H_FILES} ${SRC_CPP_FILES} ${DAEMON_SRCS}
                 ${ACCOUNTS_DBUS_SRCS} ${LOG_DBUS_SRCS} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME} PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include
                         ${PROJECT_SOURCE_DIR})

# 这里需要添加对libpython的依赖（而不是在插件中，具体原理还没太弄明白）， 否则会导致python的c
# extension（例如/usr/lib64/python3.9/lib-dynload/_posixsubprocess.cpython-39-x86_64-linux-gnu.so）无法找到libpython.so的符号表
target_link_libraries(${TARGET_NAME} PRIVATE ${PYTHON_LIBRARIES} Qt5::Network
                                             Qt5::DBus base dbus)

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_LIBEXECDIR})
