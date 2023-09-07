


include(GNUInstallDirs)

# variable definition

set(SSR_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(SSR_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
set(SSR_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(SSR_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(SSR_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(SSR_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
set(SSR_PLUGIN_ROOT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins)
set(SSR_PLUGIN_PYTHON_ROOT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/python)
set(SSR_PLUGIN_CPP_ROOT_DIR ${CMAKE_INSTALL_FULL_LIBDIR}/${PROJECT_NAME}/plugins/cpp)

if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.9)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED on)
endif()

option(ENABLE_GTEST "Enable google test" OFF)
option(USE_SYSTEMD "Use systemd or upstart" ON)
option(USE_PYTHON3 "Use Python3 as python intepreter" OFF)


# marco definition

macro(GEN_DBUS_CPP UPPER LOWER INTERFACE_PREFIX XML_PATH)

SET(${UPPER}_INTROSPECTION_XML ${XML_PATH})

SET(${UPPER}_GENERATED_CPP_FILES
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.h
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.h)


ADD_CUSTOM_COMMAND (OUTPUT ${${UPPER}_GENERATED_CPP_FILES}
    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
    COMMAND ${DBUSXX_XML2CPP} ${${UPPER}_INTROSPECTION_XML}
                                --proxy=${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.h
                                --adaptor=${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.h
    DEPENDS ${${UPPER}_INTROSPECTION_XML}
    COMMENT "Generate the proxy and adaptor for the ${LOWER}")

endmacro()


macro(GEN_PROTOCOL)
    set(SSR_PROTOCOL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ssr-protocol.hxx
                            ${CMAKE_CURRENT_BINARY_DIR}/ssr-protocol.cxx)

    add_custom_command(OUTPUT ${SSR_PROTOCOL_OUTPUT}
                       COMMAND ${XSDCXX} cxx-tree --std c++11 
                                                   --namespace-map =KS::Protocol
                                                   --type-naming ucc
                                                   --generate-serialization
                                                   --root-element-all
                                                   ${PROJECT_SOURCE_DIR}/data/ssr-protocol.xsd
                       DEPENDS ${PROJECT_SOURCE_DIR}/data/ssr-protocol.xsd
                       COMMENT "generate the c++ file by ssr-protocol.xsd")
endmacro()


macro(GEN_AND_INSTALL_PLUGIN_XML XML_IN_FILE INSTALL_DIR)
    string(REGEX REPLACE "(.+)\\..*" "\\1" XML_FILE ${XML_IN_FILE})
    add_custom_target(${XML_IN_FILE} ALL
                       COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/${XML_IN_FILE}
                                                 ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
                       COMMAND ${SED} -i -e "s/xml:lang/lang/g" ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${XML_IN_FILE})

    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
             DESTINATION ${INSTALL_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_CPP_XML XML_IN_FILE)
    GEN_AND_INSTALL_PLUGIN_XML(${XML_IN_FILE} ${SSR_PLUGIN_CPP_ROOT_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_PYTHON_XML XML_IN_FILE)
    GEN_AND_INSTALL_PLUGIN_XML(${XML_IN_FILE} ${SSR_PLUGIN_PYTHON_ROOT_DIR})
endmacro()

