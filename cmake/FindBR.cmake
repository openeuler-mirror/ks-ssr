include(GNUInstallDirs)

# variable definition

set(BR_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(BR_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
set(BR_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(BR_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(BR_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(BR_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
set(BR_PLUGIN_ROOT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins)
set(BR_PLUGIN_PYTHON_ROOT_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/python)
set(BR_PLUGIN_CPP_ROOT_DIR
    ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(ENABLE_GTEST "Enable google test" OFF)
option(USE_SYSTEMD "Use systemd or upstart" ON)
option(USE_PYTHON3 "Use Python3 as python intepreter" OFF)

# marco definition

macro(GEN_DBUS_CPP UPPER LOWER INTERFACE_PREFIX XML_PATH)

  set(${UPPER}_INTROSPECTION_XML ${XML_PATH})

  set(${UPPER}_GENERATED_CPP_FILES
      ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.h
      ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.cpp
      ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.h
      ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.cpp)

  add_custom_command(
    OUTPUT ${${UPPER}_GENERATED_CPP_FILES}
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
    COMMAND
      ${QT_DBUS_XML2CPP} ${${UPPER}_INTROSPECTION_XML}
      # --classname BR_DBus
      --proxy ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy --adaptor
      ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub
    DEPENDS ${${UPPER}_INTROSPECTION_XML}
    COMMENT "Generate the proxy and adaptor for the ${LOWER}")

endmacro()

macro(GEN_PROTOCOL)
  set(BR_PROTOCOL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/br-protocol.hxx
                         ${CMAKE_CURRENT_BINARY_DIR}/br-protocol.cxx)

  add_custom_command(
    OUTPUT ${BR_PROTOCOL_OUTPUT}
    COMMAND
      ${XSDCXX} cxx-tree --std c++11 --namespace-map =KS::Protocol --type-naming
      ucc --generate-serialization --root-element-all
      ${PROJECT_SOURCE_DIR}/data/br/br-protocol.xsd
    DEPENDS ${PROJECT_SOURCE_DIR}/data/br/br-protocol.xsd
    COMMENT "generate the c++ file by br-protocol.xsd")
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_XML XML_IN_FILE INSTALL_DIR)
  string(REGEX REPLACE "(.+)\\..*" "\\1" XML_FILE ${XML_IN_FILE})
  add_custom_target(
    ${XML_IN_FILE} ALL
    COMMAND
      ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/
      ${CMAKE_CURRENT_SOURCE_DIR}/${XML_IN_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
    COMMAND ${SED} -i -e "s/xml:lang/lang/g"
            ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${XML_IN_FILE})

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
          DESTINATION ${INSTALL_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_CPP_XML XML_IN_FILE)
  gen_and_install_plugin_xml(${XML_IN_FILE} ${BR_PLUGIN_CPP_ROOT_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_PYTHON_XML XML_IN_FILE)
  gen_and_install_plugin_xml(${XML_IN_FILE} ${BR_PLUGIN_PYTHON_ROOT_DIR})
endmacro()
