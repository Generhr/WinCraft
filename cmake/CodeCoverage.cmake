# Check supported compiler (Clang, GNU and Flang)
get_property(LANGUAGES GLOBAL PROPERTY ENABLED_LANGUAGES)

foreach(LANGUAGE ${ENABLED_LANGUAGES})
    if(${LANGUAGE} STREQUAL "RC")
        continue() # Skip the RC language
    endif()

    if("${CMAKE_${LANGUAGE}_COMPILER_ID}" MATCHES "[Cc]lang")
        if("${CMAKE_${LANGUAGE}_COMPILER_VERSION}" VERSION_LESS 3)
            message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
        endif()
    elseif(NOT "${CMAKE_${LANGUAGE}_COMPILER_ID}" MATCHES "[Cc]lang")
        message("Compiler for ${LANGUAGE} is not Clang.")
    endif()
endforeach()

set(COVERAGE_COMPILER_FLAGS
    #? -fprofile-instr-generate: This flag tells the compiler to generate instrumentation code for code coverage analysis.
    #? -fcoverage-mapping: This flag generates additional coverage mapping information to be used in code coverage reports.
    "-fprofile-instr-generate -fcoverage-mapping"
    CACHE INTERNAL ""
)

if(CMAKE_CXX_COMPILER_ID MATCHES "[Cc]lang")
    include(CheckCXXCompilerFlag)

    check_cxx_compiler_flag(-fprofile-abs-path HAVE_FPROFILE_ABS_PATH)

    if(HAVE_FPROFILE_ABS_PATH)
        #? -fprofile-abs-path: This flag, as mentioned before, stores absolute file paths in the coverage data.
        set(COVERAGE_COMPILER_FLAGS "${COVERAGE_COMPILER_FLAGS} -fprofile-abs-path")
    endif()
endif()

set(CMAKE_C_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C compiler during coverage builds." FORCE
)
set(CMAKE_CXX_FLAGS_COVERAGE
    ${COVERAGE_COMPILER_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during coverage builds." FORCE
)

set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    #? -fprofile-instr-generate (for linking): This flag is added to the linker flags to ensure proper linking with code coverage instrumentation.
    "-fprofile-instr-generate"
    CACHE STRING "Flags used for linking binaries during coverage builds." FORCE
)
set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used by the shared libraries linker during coverage builds." FORCE
)

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug" OR GENERATOR_IS_MULTI_CONFIG))
    message(WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
endif()

mark_as_advanced(CMAKE_C_FLAGS_COVERAGE CMAKE_CXX_FLAGS_COVERAGE CMAKE_EXE_LINKER_FLAGS_COVERAGE CMAKE_SHARED_LINKER_FLAGS_COVERAGE GENERATOR_IS_MULTI_CONFIG)

function(append_coverage_compiler_flags)
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_COVERAGE}"
        PARENT_SCOPE
    )
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}"
        PARENT_SCOPE
    )

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")

    message(STATUS "Appending code coverage compiler flags: ${COVERAGE_COMPILER_FLAGS}")
endfunction()

# Setup coverage for specific library
function(append_coverage_compiler_flags_to_target NAME)
    separate_arguments(_FLAG_LIST NATIVE_COMMAND "${COVERAGE_COMPILER_FLAGS}")
    target_compile_options(${NAME} PRIVATE ${_FLAG_LIST})
endfunction()
