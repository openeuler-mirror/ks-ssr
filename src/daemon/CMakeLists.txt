set(TARGET_NAME ${PROJECT_NAME}-daemon)

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
  ${CMAKE_SOURCE_DIR}/src/daemon/accounts/accounts-entity.h KS::AccountsEntity
  accounts_adaptor AccountsAdaptor)

qt5_add_dbus_adaptor(
  LOG_DBUS_SRCS ${CMAKE_SOURCE_DIR}/data/dbus/com.kylinsec.SSR.Log.xml
  ${CMAKE_SOURCE_DIR}/src/daemon/log/log-manager.h KS::Log::Manager log_adaptor
  LogAdaptor)

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/daemon/${TARGET_NAME}.zh_CN.ts")
execute_process(COMMAND lupdate-qt5 -no-obsolete ${SRC_H_FILES}
                        ${SRC_CPP_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

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

set_target_properties(
  ${TARGET_NAME}
  PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/lib64")

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
install(TARGETS ${TARGET_NAME} DESTINATION ${SSR_INSTALL_LIBEXECDIR})

install(
  PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/ks-ssr-daemon.sh"
  RENAME ${TARGET_NAME}
  DESTINATION ${SSR_INSTALL_BINDIR})
