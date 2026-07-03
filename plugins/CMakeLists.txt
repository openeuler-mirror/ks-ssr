# 指定动态库在链接时不允许出现未定义的符号， 由于目前的插件依赖于 daemon 模块的 log, 所以暂时不打开此链接选项。
# set(CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")
add_subdirectory(daemon)
add_subdirectory(ui)

if(ENABLE_BR)
  add_subdirectory(reinforcements/python)
endif()
