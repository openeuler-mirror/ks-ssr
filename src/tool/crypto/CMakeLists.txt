cmake_minimum_required(VERSION 2.8.12)

set (TARGET_NAME ks-ssr-crypto)

file(GLOB_RECURSE CRYPTO_H_FILES ./*.h)
file(GLOB_RECURSE CRYPTO_CPP_FILES ./*.cpp)


add_executable(${TARGET_NAME}
               ${CRYPTO_H_FILES}
               ${CRYPTO_CPP_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
                      lib-base)

