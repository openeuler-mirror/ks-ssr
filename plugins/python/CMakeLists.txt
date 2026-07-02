


#install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.py
#         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}/ssr)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ssr/vars.py.in ${CMAKE_BINARY_DIR}/ssr/vars.py)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ssr
         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}
         FILES_MATCHING REGEX ".*\\.(py|ini)$")

install (FILES ${CMAKE_BINARY_DIR}/ssr/vars.py
         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}/ssr)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-sysctl-ssr.conf
         RENAME "90-ssr.conf"
         DESTINATION /etc/sysctl.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-udev-ssr.rules
         RENAME "90-ssr.rules"
         DESTINATION /etc/udev/rules.d/)


GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-audit.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-config.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-external.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-network.xml.in)