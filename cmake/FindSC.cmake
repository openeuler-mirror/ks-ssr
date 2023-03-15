include(GNUInstallDirs)

set(SC_INSTALL_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR})
set(SC_INSTALL_BINDIR ${CMAKE_INSTALL_FULL_BINDIR})
set(SC_INSTALL_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME})
set(SC_INSTALL_INCLUDE ${CMAKE_INSTALL_FULL_INCLUDEDIR}/${PROJECT_NAME})
set(SC_INSTALL_SYSCONFDIR ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${PROJECT_NAME})
set(SC_INSTALL_TRANSLATIONDIR ${SC_INSTALL_DATADIR}/translations)
set(GETTEXT_PACKAGE ${PROJECT_NAME})

list(APPEND KSC_COMPILER_FLAGS -Wno-parentheses # Disable parentheses warning
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(USE_SYSTEMD "Use systemd or upstart" ON)
