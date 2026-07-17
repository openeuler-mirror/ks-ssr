add_subdirectory(config)
add_subdirectory(udev-helper)
add_subdirectory(command)

if(ENABLE_VULNERABILITY_MANAGER)
  add_subdirectory(timeshift)
endif()
