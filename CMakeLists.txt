cmake_minimum_required(VERSION 3.21...3.27 FATAL_ERROR) #? 3.21 - PROJECT_IS_TOP_LEVEL

#============  Safe Guards  ====================================================#

include(cmake/SafeGuards.cmake)

#==========  Project Details  ==================================================#

# NOTE: update this with your new project's details.

project("WinCraft"
        VERSION "0.1.0" # NOTE: Listing the version is nice here since it sets lots of useful variables.
        DESCRIPTION "Window automation tool"
        HOMEPAGE_URL "https://github.com/Onimuru/WinCraft"
        LANGUAGES CXX C # NOTE: To support external libraries, it's good practice to list both CXX and C.
)

#==========  CMake Variables  ==================================================#

message(STATUS "\n-- Configuring CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n--")

# Only do these if this is the main project, and not if it is included through add_subdirectory:
if(PROJECT_IS_TOP_LEVEL)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD
            20
            CACHE STRING "The C++ standard to use."
        )
    endif()

    # Require C++17 features like if constexpr:
    set(CMAKE_CXX_STANDARD_REQUIRED 17)

    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_EXTENSIONS OFF)
endif()

# Suppress generation of the ZERO_CHECK build target:
set(CMAKE_SUPPRESS_REGENERATION true)

set(CMAKE_COLOR_DIAGNOSTICS ON)

set(GIT_SHA
    "Unknown"
    CACHE STRING "SHA this build was generated from"
)
string(SUBSTRING "${GIT_SHA}" 0 8 GIT_SHORT_SHA)

#====  Define CMake Module Imports  ============================================#

include(cmake/Options.cmake)

include(cmake/UpdateSubmodules.cmake)

include(cmake/Cache.cmake)
enable_cache()

if(ENABLE_CPPCHECK
   OR ENABLE_CLANG_TIDY
   OR ENABLE_INCLUDE_WHAT_YOU_USE
)
    include(cmake/StaticAnalyzers.cmake)

    if(ENABLE_CPPCHECK)
        enable_cppcheck()
    endif()

    if(ENABLE_CLANG_TIDY)
        enable_clang_tidy()
    endif()

    if(ENABLE_INCLUDE_WHAT_YOU_USE)
        enable_include_what_you_use()
    endif()
endif()

if(ENABLE_SANITIZE_ADDR OR ENABLE_SANITIZE_UNDEF)
    include(cmake/Sanitize.cmake)
    add_sanitizer_flags()
endif()

if(ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    enable_ipo()
endif()

include(cmake/Warnings.cmake)

if(ENABLE_DOXYGEN)
    include(cmake/Doxygen.cmake)
    enable_doxygen()
endif()

set(VCPKG_BUILD_DOCS OFF)
set(VCPKG_BUILD_TESTS OFF)
set(VCPKG_BUILD_EXAMPLES OFF)

if(VERBOSE_OUTPUT)
    set(VCPKG_VERBOSE ON)
else()
    list(APPEND VCPKG_INSTALL_OPTIONS "--no-print-usage") #: https://learn.microsoft.com/en-us/vcpkg/commands/install
endif()

option(VCPKG_APPLOCAL_DEPS "Automatically copy dependencies into the output directory for executables." ON)

include(${CMAKE_SOURCE_DIR}/external/vcpkg/scripts/buildsystems/vcpkg.cmake)

#==========  Configure Files  ==================================================#

add_subdirectory(configure)

# Move any textures so that they are relative to the executable
file(GLOB_RECURSE COPY_FILES "res/*")
file(COPY ${COPY_FILES} DESTINATION "${CMAKE_BINARY_DIR}/bin/res")

#========== Add Source Files ==================================================#

add_subdirectory(external EXCLUDE_FROM_ALL)

set(PRECOMPILED_HEADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")

if(ENABLE_CLANG_TIDY)
    set(NO_STATIC_CODE_ANALYSIS_LIST # Add here any file you want to exclude from clang-tidy static analysis.
    )
endif()

set(HEADER_LIST # Add here any file you want to add manually which is not in include/*.
)

file(GLOB_RECURSE HEADER_FILES include/*${SUFFIX_HPPHEADER})

foreach(HEADER_FILE ${HEADER_FILES})
    if(NOT "${HEADER_FILE}" IN_LIST PRECOMPILED_HEADER_LIST AND NOT "${HEADER_FILE}" IN_LIST NO_STATIC_CODE_ANALYSIS_LIST) #~ https://gitlab.kitware.com/cmake/cmake/-/issues/22091
        list(APPEND HEADER_LIST ${HEADER_FILE})
    endif()
endforeach()

set(SOURCE_LIST # Add here any file you want to add manually which is not in src/*.cpp.
)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

foreach(SOURCE_FILE ${SOURCE_FILES})
    if(NOT "${SOURCE_FILE}" MATCHES ".*/Main.cpp" AND NOT "${SOURCE_FILE}" IN_LIST NO_STATIC_CODE_ANALYSIS_LIST)
        list(APPEND SOURCE_LIST ${SOURCE_FILE})
    endif()
endforeach()

#===========  Create Target  ===================================================#

# Delete all cmake files from previous builds
add_custom_target(clean_build_folder COMMAND git clean -d -f -x)

# Main executable
if(BUILD_HEADERS_ONLY)
    add_library(${PROJECT_NAME} INTERFACE)

    if(VERBOSE_OUTPUT)
        message(STATUS "Created ${PROJECT_NAME} interface library with the following headers:")

        foreach(HEADER IN LISTS HEADER_LIST)
            message(STATUS "* ${HEADER}")
        endforeach()
    else()
        message(STATUS "Created ${PROJECT_NAME} interface library")
    endif()
elseif(BUILD_EXECUTABLE)
    if(ENABLE_CLANG_TIDY AND NO_STATIC_CODE_ANALYSIS_LIST)
        # Create a library with the sources from `NO_STATIC_CODE_ANALYSIS_LIST` and reset its `CMAKE_CXX_CLANG_TIDY` property to avoid having clang-tidy analyze the code:
        add_library(NO_STATIC_CODE_ANALYSIS OBJECT "${NO_STATIC_CODE_ANALYSIS_LIST}")
        set_target_properties(NO_STATIC_CODE_ANALYSIS PROPERTIES CXX_CLANG_TIDY "")
        target_include_directories(NO_STATIC_CODE_ANALYSIS PRIVATE "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

        add_executable(${PROJECT_NAME} "src/Main.cpp" "$<TARGET_OBJECTS:NO_STATIC_CODE_ANALYSIS>" "${SOURCE_LIST}" "${HEADER_LIST}")
    else()
        add_executable(${PROJECT_NAME} "src/Main.cpp" "${SOURCE_LIST}" "${HEADER_LIST}")
    endif()

    if(VERBOSE_OUTPUT)
        message(STATUS "Created ${PROJECT_NAME} binary with the following sources:")
        message(STATUS "* ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")

        foreach(SOURCE IN LISTS SOURCE_LIST)
            message(STATUS "* ${SOURCE}")
        endforeach()
    else()
        message(STATUS "Created ${PROJECT_NAME} binary")
    endif()
else()
    add_library(${PROJECT_NAME} "${SOURCE_LIST}" "${HEADER_LIST}")

    if(VERBOSE_OUTPUT)
        message(STATUS "Created ${PROJECT_NAME} static library with the following sources:")

        foreach(SOURCE IN LISTS SOURCE_LIST)
            message(STATUS "* ${SOURCE}")
        endforeach()

        message(STATUS "and the following headers:")

        foreach(HEADER IN LISTS HEADER_LIST)
            message(STATUS "* ${HEADER}")
        endforeach()
    else()
        message(STATUS "Created ${PROJECT_NAME} static library")
    endif()
endif()

#========  Include Directories  ================================================#

if(BUILD_HEADERS_ONLY)
    target_include_directories(${PROJECT_NAME} INTERFACE $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
else()
    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include"
    )
endif()

#=====  Set Standard and Warnings  =============================================#

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
               CXX_STANDARD_REQUIRED 17
               CXX_EXTENSIONS NO
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

message(STATUS "* using standard C++${CMAKE_CXX_STANDARD}")

if(ENABLE_WARNINGS)
    target_set_warnings(${PROJECT_NAME})
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _AFXDLL)

#=========  Link Dependencies  =================================================#

# Precompile headers
if(NOT ENABLE_CLANG_TIDY)
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PRECOMPILED_HEADER_LIST}>")
endif()

find_package(Microsoft.GSL CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft.GSL::GSL)

#===========  Alias Targets  ===================================================#

if(BUILD_EXECUTABLE)
    add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
else()
    add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif()

#========= Unit Testing Setup =================================================#

if(PROJECT_IS_TOP_LEVEL)
    if(ENABLE_TESTING)
        if(ENABLE_CODE_COVERAGE)
            include(cmake/CodeCoverage.cmake)

            append_coverage_compiler_flags()

            setup_target_for_coverage_lcov(
                NAME
                coverage
                EXECUTABLE
                ctest
                EXCLUDE
                "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*"
                "${CMAKE_CURRENT_SOURCE_DIR}/external/*"
                "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/test/*"
                "${CMAKE_CURRENT_SOURCE_DIR}/build/*"
                "${CMAKE_CURRENT_SOURCE_DIR}/docs/*"
            )
        endif()

        add_library(TESTING_LIBRARY INTERFACE)
        target_sources(TESTING_LIBRARY INTERFACE "${SOURCE_LIST}" "${HEADER_LIST}") # NOTE: Including header files here helps IDEs but is not required.
        target_include_directories(TESTING_LIBRARY INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
        set_target_properties(
            ${TESTING_LIBRARY}
            PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
                       CXX_STANDARD_REQUIRED 17
                       CXX_EXTENSIONS NO
        )

        if(ENABLE_CLANG_TIDY AND CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            target_precompile_headers(TESTING_LIBRARY INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${PRECOMPILED_HEADER_LIST}>")
        elseif(NOT ENABLE_CLANG_TIDY)
            target_precompile_headers(TESTING_LIBRARY INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${PRECOMPILED_HEADER_LIST}>")
        endif()

        enable_testing()

        find_package(GTest CONFIG REQUIRED)
        include(GoogleTest)

        mark_as_advanced(
            BUILD_GMOCK
            BUILD_GTEST
            BUILD_SHARED_LIBS
            gmock_build_tests
            gtest_build_samples
            gtest_build_tests
            gtest_disable_pthreads
            gtest_force_shared_crt
            gtest_hide_internal_symbols
        )

        add_subdirectory(test)
    endif()
endif()

#==============  Install  ======================================================#

include(GNUInstallDirs)

# Determine the installation directory for headers and libraries (the CMAKE_INSTALL_* variables are defined by the GNUInstallDirs module):
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}")
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_BINDIR}")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")
