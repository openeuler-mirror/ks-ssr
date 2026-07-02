cmake_minimum_required(VERSION 3.2)

enable_testing()

set (TARGET_NAME ks-ssr-tests)

file(GLOB_RECURSE TEST_H_FILES ./*.h)
file(GLOB_RECURSE TEST_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${TEST_H_FILES}
               ${TEST_CPP_FILES})

target_include_directories(${TARGET_NAME} PUBLIC
                           ${GTEST_INCLUDE_DIRS})           

target_link_libraries(${TARGET_NAME} PRIVATE
                      ${GTEST_LIBRARIES}
                      lib-base)

add_test(NAME test 
        COMMAND ${TARGET_NAME})