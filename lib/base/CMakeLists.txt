set(TARGET_NAME base)

add_definitions("-DSQLITE_HAS_CODEC")

file(GLOB_RECURSE BASE_H_FILES ./*.h)
file(GLOB_RECURSE BASE_CPP_FILES ./*.cpp)

set(ALL_FILES ${BASE_H_FILES} ${BASE_CPP_FILES})

find_path(
  LIBHPDF_INCLUDE_DIR
  NAMES hpdf.h
  PATHS /usr/include /usr/local/include)

find_library(
  LIBHPDF_LIBRARY
  NAMES hpdf
  PATHS /usr/lib /usr/local/lib)

if(NOT LIBHPDF_INCLUDE_DIR OR NOT LIBHPDF_LIBRARY)
  message(FATAL_ERROR "Could not find libharu library and/or headers")
else()
  message(STATUS "Found libharu: ${LIBHPDF_LIBRARY}")
  message(STATUS "Found libharu headers: ${LIBHPDF_INCLUDE_DIR}")
endif()

# 更新ts文件
set(TS_FILES
    "${PROJECT_SOURCE_DIR}/translations/lib/${PROJECT_NAME}-${TARGET_NAME}.zh_CN.ts"
)
execute_process(COMMAND lupdate-qt5 -no-obsolete ${ALL_FILES} -ts ${TS_FILES})

# 生成qm文件，qt_create_translation会导致cmake clean把ts文件删掉
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${TARGET_NAME} STATIC ${ALL_FILES} ${QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${PROJECT_SOURCE_DIR}
         ${PROJECT_SOURCE_DIR}/include
         ${PROJECT_BINARY_DIR}
         ${KLOG_QT5_INCLUDE_DIRS}
         ${LIBNOTIFY_INCLUDE_DIRS}
         ${CRYPTOPP_INCLUDE_DIRS}
         ${SQLCIPHER_INCLUDE_DIRS}
         ${Qt5Core_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} PRIVATE ${CRYPTOPP_INCLUDE_DIRS}
                                                  ${LIBHPDF_INCLUDE_DIR})

target_link_libraries(
  ${TARGET_NAME}
  PUBLIC Qt5::Core Qt5::Network ${SQLCIPHER_LIBRARIES} ${LIBNOTIFY_LIBRARIES}
         ${KLOG_QT5_LIBRARIES} ${LIBHPDF_LIBRARY})

target_link_libraries(${TARGET_NAME} PRIVATE ${CRYPTOPP_LIBRARIES})

install(FILES ${QM_FILES} DESTINATION ${SSR_INSTALL_TRANSLATIONDIR})
