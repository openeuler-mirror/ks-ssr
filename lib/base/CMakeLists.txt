cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME lib-base)

add_compile_options("-DSQLITE_HAS_CODEC")

file(GLOB_RECURSE BASE_H_FILES ./*.h)
file(GLOB_RECURSE BASE_CPP_FILES ./*.cpp)

set(ALL_FILES ${BASE_H_FILES} ${BASE_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}
         ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR}
         ${KLOG_QT5_INCLUDE_DIRS}
         ${CRYPTOPP_INCLUDE_DIRS}
         ${SQLCIPHER_INCLUDE_DIRS}
         ${Qt5Core_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} PRIVATE ${CRYPTOPP_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME} PUBLIC Qt5::Core ${SQLCIPHER_LIBRARIES}
                        ${KLOG_QT5_LIBRARIES})

target_link_libraries(${TARGET_NAME} PRIVATE ${CRYPTOPP_LIBRARIES})
