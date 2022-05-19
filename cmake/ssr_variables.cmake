


include(GNUInstallDirs)

set(SSR_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(SSR_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
set(SSR_INSTALL_INCLUDE ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
set(SSR_INSTALL_DATADIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})
set(SSR_INSTALL_SYSCONFDIR /${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME})
set(SSR_PLUGIN_ROOT_DIR ${SSR_INSTALL_LIBDIR}/${PROJECT_NAME}/plugins)
set(SSR_PLUGIN_PYTHON_ROOT_DIR ${SSR_INSTALL_LIBDIR}/${PROJECT_NAME}/plugins/python)
set(SSR_PLUGIN_CPP_ROOT_DIR ${SSR_INSTALL_LIBDIR}/${PROJECT_NAME}/plugins/cpp)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

option(USE_PYTHON3 "Use Python3 as python intepreter" ON)


# Platform-specific compiler/linker flags.
list(APPEND SSR_COMPILER_FLAGS
  -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
  -fPIC                           # Generate position-independent code for shared libraries
  -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
  -pipe                           # Use pipes rather than temporary files for communication between build stages
  -Wall                           # Enable all warnings
  -Werror                         # Treat warnings as errors
  -Wno-unused-value               # Disable unused value warning
  -Wno-missing-braces             # Disable missing braces warning
  -Wno-deprecated-declarations    # Disable deprecated declarations warning
  -Wno-parentheses                # Disable parentheses warning
  )

list(APPEND SSR_COMPILER_FLAGS    # for C++
  #-fno-rtti                       # Disable real-time type information
  -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
  )

if(COMPILER_GCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
  list(APPEND SSR_COMPILER_FLAGS -Wno-format-truncation) # deal with a bug in gcc8
endif()


list(APPEND SSR_COMPILER_FLAGS_DEBUG
  -O0                             # Disable optimizations
  -g                              # Generate debug information
  )

list(APPEND SSR_COMPILER_FLAGS_RELEASE
  -O3                             # Optimize for maximum speed
  -fdata-sections                 # Enable linker optimizations to improve locality of reference for data sections
  -ffunction-sections             # Enable linker optimizations to improve locality of reference for function sections
  )


list(APPEND SSR_LINKER_FLAGS
  -fPIC                              # Generate position-independent code for shared libraries
  -Wl,--fatal-warnings               # Treat warnings as errors
  -Wno-deprecated-declarations       # Disable deprecated declarations warning
  #-Wl,-z,noexecstack                # Mark the stack as non-executable (security feature)
  #-Wl,-z,relro                      # Mark relocation sections as read-only (security feature)
  )

list(APPEND SSR_LINKER_FLAGS_RELEASE
  -Wl,-O1                         # Enable linker optimizations
  -Wl,--as-needed                 # Only link libraries that export symbols used by the binary
  -Wl,--gc-sections               # Remove unused code resulting from -fdata-sections and -function-sections
  )


include(CheckCXXCompilerFlag)
