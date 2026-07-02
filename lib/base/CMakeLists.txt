cmake_minimum_required(VERSION 2.8.12)

set (TARGET_NAME lib-base)

file(GLOB_RECURSE BASE_H_FILES ./*.h)
file(GLOB_RECURSE BASE_CPP_FILES ./*.cpp)

set (ALL_FILES ${BASE_H_FILES} ${BASE_CPP_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES})

target_include_directories(${TARGET_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${GIOMM_INCLUDE_DIRS}
    ${KLOG_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
    ${CRYPTOPP_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS})


target_link_libraries(${TARGET_NAME} PUBLIC
    ${GIOMM_LIBRARIES}
    ${KLOG_LIBRARIES}
    ${FMT_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    pthread)

target_link_libraries(${TARGET_NAME} PRIVATE
                      ${CRYPTOPP_LIBRARIES})

