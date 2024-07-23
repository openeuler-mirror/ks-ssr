set(TARGET_NAME spm)

add_definitions(-fPIC)

# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")

pkg_search_module(LIBDNF REQUIRED libdnf)

version_to_hash(${LIBDNF_VERSION} LIBDNF_VERSION_HASH)
message(STATUS "libdnf version: ${LIBDNF_VERSION}")
add_definitions(-DKS_DEP_LIBDNF_VERSION=${LIBDNF_VERSION_HASH})

file(GLOB_RECURSE DNF_H_FILES ./*.h)
file(GLOB_RECURSE DNF_CPP_FILES ./*.cpp)

set(ALL_FILES ${DNF_H_FILES} ${DNF_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE ${LIBDNF_LIBRARIES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}
          ${PROJECT_SOURCE_DIR}/include ${KLOG_QT5_INCLUDE_DIRS}
          ${LIBDNF_INCLUDE_DIRS})
