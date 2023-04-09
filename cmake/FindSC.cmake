include(GNUInstallDirs)

set(SC_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(SC_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(SC_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
set(SC_INSTALL_TRANSLATIONDIR ${SC_INSTALL_DATADIR}/translations)
set(GETTEXT_PACKAGE ${PROJECT_NAME})

set(SC_BOX_MOUNT_DIR /box)
set(SC_BOX_MOUNT_DATADIR ${SC_INSTALL_DATADIR}/box)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(USE_SYSTEMD "Use systemd or upstart" ON)
