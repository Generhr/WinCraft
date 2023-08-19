cmake_minimum_required(VERSION 3.21...3.27 FATAL_ERROR)

#==========  Project Details  ==================================================#

project(${CMAKE_PROJECT_NAME}Tests LANGUAGES CXX C)

#==========  Configure Files  ==================================================#

# Move the ini file so that it is relative to the executable
file(COPY "../cfg/Settings.ini" DESTINATION "${CMAKE_BINARY_DIR}/test")

#========== Add Unit Test(s) ==================================================#

file(GLOB_RECURSE TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if(USE_GOOGLE_MOCK)
    set(GOOGLE_MOCK_LIBRARIES GTest::gmock_main)
endif()

set(TEST_LIBRARIES TESTING_LIBRARY GTest::gtest_main ${GOOGLE_MOCK_LIBRARIES})

foreach(TEST_FILE ${TEST_FILES})
    string(REGEX REPLACE "(.*/)([a-zA-Z0-9_ ]+)(\.cpp)" "\\2" TEST_NAME ${TEST_FILE})

    if(${TEST_NAME} STREQUAL "CMyWindow")
        add_executable(${TEST_NAME} ${TEST_FILE} "../res/resource.rc")
    else()
        add_executable(${TEST_NAME} ${TEST_FILE})
    endif()

    message(STATUS "Created ${TEST_NAME} binary")

    set_target_properties(
        ${TEST_NAME}
        PROPERTIES CXX_STANDARD ${CMAKE_CXX_STANDARD}
                   CXX_STANDARD_REQUIRED 17
                   CXX_EXTENSIONS NO
                   RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}
                   EXCLUDE_FROM_ALL TRUE
                   EXCLUDE_FROM_DEFAULT_BUILD TRUE
    )

    message(STATUS "* using standard C++${CMAKE_CXX_STANDARD}")

    if(ENABLE_WARNINGS)
        target_set_warnings(${TEST_NAME})
    endif()

    target_link_libraries(${TEST_NAME} PRIVATE ${TEST_LIBRARIES})

    gtest_discover_tests(
        ${TEST_NAME}
        PROPERTIES
        TIMEOUT 10
    )

    list(APPEND TEST_NAMES "${TEST_NAME}")
endforeach()

add_custom_target(unit_tests DEPENDS ${TEST_NAMES})

if(ENABLE_CODE_COVERAGE)
    if(CMAKE_GENERATOR MATCHES "Ninja Multi")
        set(CURRENT_BINARY_DIR "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
    elseif(CMAKE_GENERATOR MATCHES "Ninja")
        set(CURRENT_BINARY_DIR "${PROJECT_BINARY_DIR}")
    endif()

    foreach(TEST_NAME ${TEST_NAMES})
        add_custom_command(
            TARGET unit_tests
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=${TEST_NAME}.profraw $<TARGET_FILE:${TEST_NAME}> > NUL 2>&1
            COMMAND llvm-profdata merge -sparse "${TEST_NAME}.profraw" -o "${TEST_NAME}.profdata"
            COMMAND llvm-cov show $<TARGET_FILE:${TEST_NAME}> -instr-profile="${TEST_NAME}.profdata" > "${CMAKE_BINARY_DIR}/coverage_${TEST_NAME}.txt" -j ${PROCESSOR_COUNT}
            WORKING_DIRECTORY "${CURRENT_BINARY_DIR}"
            COMMENT "Generating code coverage report for ${TEST_NAME}.exe"
        )
    endforeach()
endif()

message(STATUS "Created `unit_tests` build target that will build all tests found in ${CMAKE_CURRENT_SOURCE_DIR} (${TEST_NAMES})")
