


#install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.py
#         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}/ssr)


configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ssr/vars.py.in ${CMAKE_BINARY_DIR}/ssr/vars.py)

install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ssr
         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}
         FILES_MATCHING REGEX ".*\\.(py|ini)$")

install (FILES ${CMAKE_BINARY_DIR}/ssr/vars.py
         DESTINATION ${SSR_PLUGIN_PYTHON_ROOT_DIR}/ssr)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-ssr-network-sysctl.conf
         RENAME "90-ssr-network.conf"
         DESTINATION /etc/sysctl.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-ssr-external-udev.rules
         RENAME "90-ssr-external.rules"
         DESTINATION /etc/udev/rules.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssr-external-profile.sh
         RENAME "ssr-external.sh"
         DESTINATION /etc/profile.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssr-config-profile.sh
         RENAME "ssr-config.sh"
         DESTINATION /etc/profile.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-ssr-config-sysctl.conf
         RENAME "90-ssr-config.conf"
         DESTINATION /etc/sysctl.d/)

install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-ssr-config-resource.conf
         RENAME "90-ssr-config.conf"
         DESTINATION /etc/security/limits.d/)


GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-audit.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-config.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-external.xml.in)
GEN_AND_INSTALL_PLUGIN_PYTHON_XML(ssr-plugin-network.xml.in)
