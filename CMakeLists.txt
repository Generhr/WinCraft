cmake_minimum_required(VERSION 3.27...3.27 FATAL_ERROR) #? 3.27 - SKIP_LINTING

#==========  Project Details  ==================================================#

project("WinCraft"
        VERSION "0.4.2"
        DESCRIPTION "Window automation tool"
        HOMEPAGE_URL "https://github.com/Onimuru/WinCraft"
        LANGUAGES CXX C
)

#============  Safe Guards  ====================================================#

include(cmake/SafeGuards.cmake)
variable_watch(CMAKE_BUILD_TYPE readonly_guard)

#==========  CMake Variables  ==================================================#

message(STATUS "\n-- Configuring CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n--")

if(PROJECT_IS_TOP_LEVEL)
    if(NOT DEFINED CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD
            20
            CACHE STRING "The C++ standard to use."
        )
    endif()

    set(CMAKE_CXX_STANDARD_REQUIRED 17)

    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_C_EXTENSIONS OFF)
endif()

set(CMAKE_SUPPRESS_REGENERATION ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_COLOR_DIAGNOSTICS ON)

include(ProcessorCount)
ProcessorCount(PROCESSOR_COUNT)

if(PROCESSOR_COUNT EQUAL 0)
    set(PROCESSOR_COUNT 1)
endif()

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_TOLOWER)
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_TOUPPER)

set(GIT_SHA
    "Unknown"
    CACHE STRING "SHA this build was generated from"
)
string(SUBSTRING "${GIT_SHA}" 0 8 GIT_SHORT_SHA)

#=======  Manual File Additions  ===============================================#

set(SKIP_LINTING_LIST)

set(PRECOMPILED_HEADER_LIST "${CMAKE_CURRENT_SOURCE_DIR}/include/pch.h")

set(MANUAL_HEADER_LIST)

# Include the .rc file directly as a source file for the project, CMake is flexible enough to consider any file with a recognized source file extension (like .cpp, .c, .rc, etc.) as part of the source code. Including the .rc file in this way tells CMake to compile it as part of your project without needing to manually compile it into a .res resource file.
set(MANUAL_SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/res/resource.rc")

#====  Define CMake Module Imports  ============================================#

include(CMakeParseArguments)

include(cmake/Options.cmake)

include(cmake/Utilities.cmake)

include(cmake/UpdateSubmodules.cmake)

include(cmake/Cache.cmake)
enable_cache()

if(ENABLE_STATIC_ANALYZERS)
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

#=========  Compiler/OS Fixes  =================================================#

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    if(WIN32)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-fcolor-diagnostics> $<$<COMPILE_LANGUAGE:CXX>:-fcolor-diagnostics>)
    else()
        add_compile_options(-fcolor-diagnostics)
    endif()

    set(WINDOWS_HEADERS_NEEDED_BY_CLANG_LIST "$ENV{EXTERNAL_INCLUDE}")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(WIN32)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-fdiagnostics-color=always> $<$<COMPILE_LANGUAGE:CXX>:-fdiagnostics-color=always>)
    else()
        add_compile_options(-fdiagnostics-color=always)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(MSVC_VERSION GREATER 1900)
        add_compile_options(/diagnostics:column)
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
else()
    message(STATUS "No colored compiler diagnostic set for '${CMAKE_CXX_COMPILER_ID}' compiler.")
endif()

#==========  Configure Files  ==================================================#

configure_file("configure/config.h.in" "${CMAKE_CURRENT_BINARY_DIR}/configured_files/include/config.h" ESCAPE_QUOTES)

# Move the ini file so that it is relative to the executable
file(COPY "cfg/Settings.ini" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/bin")

#========== Add Source Files ==================================================#

add_subdirectory(external EXCLUDE_FROM_ALL)

file(GLOB_RECURSE HEADER_LIST include/*)
list(APPEND HEADER_LIST ${MANUAL_HEADER_LIST})
list(REMOVE_DUPLICATES HEADER_LIST)
list(REMOVE_ITEM HEADER_LIST ${PRECOMPILED_HEADER_LIST})

file(GLOB_RECURSE SOURCE_LIST src/*.cpp)
list(APPEND SOURCE_LIST ${MANUAL_SOURCE_LIST})
list(REMOVE_DUPLICATES SOURCE_LIST)
list(REMOVE_ITEM SOURCE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")

foreach(SOURCE_FILE IN LISTS SKIP_LINTING_LIST)
    set_source_files_properties(${SOURCE_FILE} PROPERTIES SKIP_LINTING ON)
endforeach()

#===========  Create Target  ===================================================#

# Main executable
add_executable(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp" "${SOURCE_LIST}" "${HEADER_LIST}")

if(VERBOSE_OUTPUT)
    message(STATUS "Created ${PROJECT_NAME} binary with the following sources:")
    message(STATUS "* ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp")

    foreach(SOURCE IN LISTS SOURCE_LIST)
        message(STATUS "* ${SOURCE}")
    endforeach()
else()
    message(STATUS "Created ${PROJECT_NAME} binary")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${WINDOWS_HEADERS_NEEDED_BY_CLANG_LIST}" "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

# Precompile headers
if(NOT ENABLE_CLANG_TIDY OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    target_precompile_headers(${PROJECT_NAME} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:${PRECOMPILED_HEADER_LIST}>")
endif()

#=====  Set Standard and Warnings  =============================================#

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
               CXX_STANDARD_REQUIRED 17
               CXX_EXTENSIONS NO
               RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin"
               LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
               ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib"
)

message(STATUS "* using standard C++${CMAKE_CXX_STANDARD}")

if(ENABLE_WARNINGS)
    target_set_warnings(${PROJECT_NAME})
endif()

#========  Compile Definitions  ================================================#

# Target
target_compile_definitions(${PROJECT_NAME} PRIVATE "$<$<CONFIG:DEBUG>:-D${PROJECT_NAME_TOUPPER}_DEBUG>")

# Global
add_compile_definitions(_AFXDLL)

#=========  Link Dependencies  =================================================#

find_package(Microsoft.GSL CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Microsoft.GSL::GSL)

find_path(SIMPLEINI_INCLUDE_DIRS "ConvertUTF.c")
target_include_directories(${PROJECT_NAME} PRIVATE ${SIMPLEINI_INCLUDE_DIRS})

#===========  Alias Targets  ===================================================#

add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

#==============  Install  ======================================================#

include(GNUInstallDirs)

# Determine the installation directory for headers and libraries (the CMAKE_INSTALL_* variables are defined by the GNUInstallDirs module):
set(INSTALL_INCLUDE_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${PROJECT_VERSION}")
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_BINDIR}")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_LIBDIR}")

#========= Unit Testing Setup =================================================#

if(PROJECT_IS_TOP_LEVEL)
    if(ENABLE_TESTING)
        include(CTest)

        add_library(TESTING_LIBRARY INTERFACE)

        if(ENABLE_CODE_COVERAGE)
            include(cmake/CodeCoverage.cmake)
            append_coverage_compiler_flags_to_target(TESTING_LIBRARY)
        endif()

        target_sources(TESTING_LIBRARY INTERFACE "${SOURCE_LIST}" "${HEADER_LIST}")
        target_include_directories(TESTING_LIBRARY INTERFACE "${WINDOWS_HEADERS_NEEDED_BY_CLANG_LIST}" "${CMAKE_CURRENT_SOURCE_DIR}/include")

        if(NOT ENABLE_CLANG_TIDY OR CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
            target_precompile_headers(TESTING_LIBRARY INTERFACE "$<$<COMPILE_LANGUAGE:CXX>:${PRECOMPILED_HEADER_LIST}>")
        endif()

        set_target_properties(
            TESTING_LIBRARY
            PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
                       CXX_STANDARD_REQUIRED 17
                       CXX_EXTENSIONS NO
        )

        target_compile_definitions(TESTING_LIBRARY INTERFACE "${PROJECT_NAME_TOUPPER}_TEST")

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

#==============  Doxygen  ======================================================#

if(ENABLE_DOXYGEN)
    include(cmake/Doxygen.cmake)
    enable_doxygen()
endif()
