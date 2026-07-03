add_subdirectory(daemon)
add_subdirectory(ui)

if(ENABLE_BR)
  add_subdirectory(reinforcements/python)
endif()
