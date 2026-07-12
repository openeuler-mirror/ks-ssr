install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/src/tool/timeshift/dracuts/scripts/
  DESTINATION ${SSR_INSTALL_LIBDIR}/${PROJECT_NAME}/timeshift/dracuts/scripts/
  USE_SOURCE_PERMISSIONS)

install(
  DIRECTORY
    ${PROJECT_SOURCE_DIR}/src/tool/timeshift/dracuts/modules.d_6/99ksssr/
  DESTINATION /usr/share/dracut/modules.d/99ksssr
  USE_SOURCE_PERMISSIONS)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/src/tool/timeshift/dracuts/modules.d/99ksssr/
  DESTINATION /usr/lib/dracut/modules.d/99ksssr
  USE_SOURCE_PERMISSIONS)

install(
  DIRECTORY ${PROJECT_SOURCE_DIR}/src/tool/timeshift/python/
  DESTINATION ${SSR_INSTALL_LIBDIR}/ks-ssr/timeshift/
  FILES_MATCHING
  PATTERN "*.py")

install(PROGRAMS ${PROJECT_SOURCE_DIR}/src/tool/timeshift/ks-ssr-timeshift
        DESTINATION ${SSR_INSTALL_BINDIR}/)
