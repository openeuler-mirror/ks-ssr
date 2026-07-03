if(ENABLE_BR)
  add_subdirectory(br)
endif()

if(ENABLE_DEVICE_MANAGER)
  add_subdirectory(dm)
endif()

if(ENABLE_KSS)
  add_subdirectory(fp)
  add_subdirectory(tp)
endif()

if(ENABLE_LOG_AUDIT)
  add_subdirectory(log)
endif()

if(ENABLE_PRIVATE_BOX)
  add_subdirectory(private-box)
endif()

if(ENABLE_REMOTE_MANAGER)
  add_subdirectory(remote)
endif()

if(ENABLE_TOOL_BOX)
  add_subdirectory(tool-box)
endif()

if(ENABLE_VULNERABILITY_MANAGER)
  add_subdirectory(vulnerability)
endif()
