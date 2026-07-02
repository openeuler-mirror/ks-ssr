


macro (list_join)
foreach(elem IN LISTS ${ARGV0})
    string(APPEND ${ARGV2} ${ARGV1} ${elem})
endforeach()
endmacro()

macro(SET_COMMON_COMPILER_FLAGS)
list_join(SSR_COMPILER_FLAGS " " CMAKE_CXX_FLAGS)
list_join(SSR_COMPILER_FLAGS_DEBUG " " CMAKE_CXX_FLAGS_DEBUG)
list_join(SSR_COMPILER_FLAGS_RELEASE " " CMAKE_CXX_FLAGS_RELEASE)
endmacro()

macro(SET_COMMON_EXE_LINKER_FLAGS)
list_join(SSR_LINKER_FLAGS " " CMAKE_EXE_LINKER_FLAGS)
list_join(SSR_LINKER_FLAGS_DEBUG " " CMAKE_EXE_LINKER_FLAGS_DEBUG)
list_join(SSR_LINKER_FLAGS_RELEASE " " CMAKE_EXE_LINKER_FLAGS_RELEASE)
endmacro()

macro(SET_COMMON_SHARED_LINKER_FLAGS)
list_join(SSR_LINKER_FLAGS " " CMAKE_SHARED_LINKER_FLAGS)
list_join(SSR_LINKER_FLAGS_DEBUG " " CMAKE_SHARED_LINKER_FLAGS_DEBUG)
list_join(SSR_LINKER_FLAGS_RELEASE " " CMAKE_SHARED_LINKER_FLAGS_RELEASE)
endmacro()



macro(GEN_DBUS_CPP UPPER LOWER INTERFACE_PREFIX XML_PATH)

SET (${UPPER}_INTROSPECTION_XML ${XML_PATH})

SET (${UPPER}_GENERATED_CPP_FILES
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.cpp
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_stub.h
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_common.cpp
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_common.h
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.cpp
    ${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus_proxy.h
)

ADD_CUSTOM_COMMAND (OUTPUT ${${UPPER}_GENERATED_CPP_FILES}
                    COMMAND mkdir -p ${CMAKE_BINARY_DIR}/generated/
                    COMMAND ${GDBUS_CODEGEN} --generate-cpp-code=${CMAKE_BINARY_DIR}/generated/${LOWER}_dbus
                            --interface-prefix=${INTERFACE_PREFIX}
                            ${${UPPER}_INTROSPECTION_XML}
                    DEPENDS ${${UPPER}_INTROSPECTION_XML}
                    COMMENT "Generate the stub for the ${LOWER}")

endmacro()


macro(GEN_PROTOCOL)
    set(SSR_PROTOCOL_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ssr-protocol.hxx
                            ${CMAKE_CURRENT_BINARY_DIR}/ssr-protocol.cxx)

    add_custom_command(OUTPUT ${SSR_PROTOCOL_OUTPUT}
                       COMMAND ${XSDCXX} cxx-tree --std c++11 
                                                   --namespace-map =Kiran::Protocol
                                                   --type-naming ucc
                                                   --generate-serialization
                                                   --root-element-all
                                                   ${PROJECT_SOURCE_DIR}/data/ssr-protocol.xsd
                       DEPENDS ${PROJECT_SOURCE_DIR}/data/ssr-protocol.xsd
                       COMMENT "generate the c++ file by ssr-protocol.xsd")
endmacro()


macro(GEN_AND_INSTALL_PLUGIN_XML XML_IN_FILE INSTALL_DIR)
    string(REGEX REPLACE "(.+)\\..*" "\\1" XML_FILE ${XML_IN_FILE})
    execute_process(COMMAND ${INTLTOOL-MERGE} -x ${PROJECT_SOURCE_DIR}/po/
                                                 ${CMAKE_CURRENT_SOURCE_DIR}/${XML_IN_FILE}
                                                 ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE})

    execute_process(COMMAND ${SED} -i -e "s/xml:lang/lang/g" ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE})


    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${XML_FILE}
             DESTINATION ${INSTALL_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_CPP_XML XML_IN_FILE)
    GEN_AND_INSTALL_PLUGIN_XML(${XML_IN_FILE} ${SSR_PLUGIN_CPP_ROOT_DIR})
endmacro()

macro(GEN_AND_INSTALL_PLUGIN_PYTHON_XML XML_IN_FILE)
    GEN_AND_INSTALL_PLUGIN_XML(${XML_IN_FILE} ${SSR_PLUGIN_PYTHON_ROOT_DIR})
endmacro()