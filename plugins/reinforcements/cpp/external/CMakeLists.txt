cmake_minimum_required(VERSION 2.8.12)

set(TARGET_NAME external)

file(GLOB_RECURSE EXTERNAL_H_FILES ./*.h)
file(GLOB_RECURSE EXTERNAL_CPP_FILES ./*.cpp)

add_library(${TARGET_NAME} SHARED ${EXTERNAL_H_FILES} ${EXTERNAL_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE lib-base)

gen_and_install_plugin_cpp_xml(br-plugin-external.xml.in)

install(TARGETS ${TARGET_NAME}
        DESTINATION $SSR_BR_PLUGIN_ROOT_DIR_ROOT_DIR}/cpp/)
