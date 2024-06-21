include(GNUInstallDirs)

set(SSR_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(SSR_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
set(SSR_INSTALL_TRANSLATIONDIR ${SSR_INSTALL_DATADIR}/translations)
set(SSR_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(SSR_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
set(SSR_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(SSR_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(SSR_INSTALL_PLUGINDIR ${SSR_INSTALL_LIBDIR}/${PROJECT_NAME}/plugins)
set(SSR_INSTALL_DAEMON_PLUGINDIR ${SSR_INSTALL_PLUGINDIR}/daemon)
set(SSR_INSTALL_UI_PLUGINDIR ${SSR_INSTALL_PLUGINDIR}/gui)

set(SSR_BR_PLUGIN_ROOT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins)
set(SSR_BR_PLUGIN_CPP_ROOT_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/cpp)
set(SSR_BR_PLUGIN_PYTHON_ROOT_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/python)
set(SSR_BR_PLUGIN_BASH_ROOT_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/bash)

set(SSR_BOX_MOUNT_DIR /box)
set(SSR_BOX_MOUNT_DATADIR ${SSR_INSTALL_DATADIR}/box)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(USE_SYSTEMD "Use systemd or upstart" ON)
option(USE_PYTHON3 "Use Python3 as python intepreter" OFF)

macro(gen_protocol)
  set(BR_PROTOCOL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/br-protocol.hxx
                         ${CMAKE_CURRENT_BINARY_DIR}/br-protocol.cxx)

  add_custom_command(
    OUTPUT ${BR_PROTOCOL_OUTPUT}
    COMMAND
      ${XSDCXX} cxx-tree --std c++11 --namespace-map =KS::Protocol --type-naming
      ucc --generate-serialization --root-element-all
      ${PROJECT_SOURCE_DIR}/data/br-protocol.xsd
    DEPENDS ${PROJECT_SOURCE_DIR}/data/br-protocol.xsd
    COMMENT "generate the c++ file by br-protocol.xsd")
endmacro()

# find_package 和 pkg_search_module 会使能 ${PACKAGE}_VERSION 变量， 变量的值是 "1.2.3"
# 格式的字符串 此函数将通过算法 1 << 16 | 2 << 8 | 3 的方式将版本号转换为 32 位的整数
function(version_to_hash version_string version_hashed)

  # 使用字符串替换分割版本号
  string(REPLACE "." ";" version_parts ${version_string})
  list(GET version_parts 0 major)
  list(GET version_parts 1 minor)
  list(GET version_parts 2 patch)

  # 确保版本号是数字
  if(NOT major MATCHES "^([0-9]+)$"
     OR NOT minor MATCHES "^([0-9]+)$"
     OR NOT patch MATCHES "^([0-9]+)$")
    message(
      WARNING
        "Invalid version format. Please use 'X.Y.Z' where X, Y, and Z are integers."
    )
  endif()

  # 计算哈希值
  math(EXPR hashed_version "${major} << 16 | ${minor} << 8 | ${patch}")
  set(${version_hashed}
      ${hashed_version}
      PARENT_SCOPE)

endfunction()
