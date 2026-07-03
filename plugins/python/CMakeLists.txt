# install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.py DESTINATION
# ${SSR_BR_PLUGIN_PYTHON_ROOT_DIR}/br)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/br/vars.py.in
               ${CMAKE_BINARY_DIR}/br/vars.py)

install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/br
  DESTINATION ${SSR_BR_PLUGIN_PYTHON_ROOT_DIR}
  FILES_MATCHING
  REGEX ".*\\.(py|ini)$")

install(FILES ${CMAKE_BINARY_DIR}/br/vars.py
        DESTINATION ${SSR_BR_PLUGIN_PYTHON_ROOT_DIR}/br)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-br-network-sysctl.conf
  RENAME "90-br-network.conf"
  DESTINATION /etc/sysctl.d/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-br-external-udev.rules
  RENAME "90-br-external.rules"
  DESTINATION /etc/udev/rules.d/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/br-external-profile.sh
  RENAME "br-external.sh"
  DESTINATION /etc/profile.d/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-br-config-sysctl.conf
  RENAME "90-br-config.conf"
  DESTINATION /etc/sysctl.d/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/90-br-config-resource.conf
  RENAME "90-br-config.conf"
  DESTINATION /etc/security/limits.d/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/98-br-config.gschema.override
  RENAME "98-br-config.gschema.override"
  DESTINATION /usr/share/glib-2.0/schemas/)

install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/br-blacklist.conf
  RENAME "br-blacklist.conf"
  DESTINATION /etc/modprobe.d/)

gen_and_install_plugin_python_xml(br-plugin-audit.xml.in)
gen_and_install_plugin_python_xml(br-plugin-config.xml.in)
gen_and_install_plugin_python_xml(br-plugin-external.xml.in)
gen_and_install_plugin_python_xml(br-plugin-network.xml.in)
