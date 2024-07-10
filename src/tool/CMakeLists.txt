add_subdirectory(config)
add_subdirectory(crypto)

if(ENABLE_VULNERABILITY_MANAGER)
  add_subdirectory(command)
  add_subdirectory(timeshift)
endif()
