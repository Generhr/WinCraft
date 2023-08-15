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

set(TEST_LIBRARIES GTest::gtest_main ${GOOGLE_MOCK_LIBRARIES} TESTING_LIBRARY)

foreach(TEST_FILE ${TEST_FILES})
    string(REGEX REPLACE "(.*/)([a-zA-Z0-9_ ]+)(\.cpp)" "\\2" TEST_NAME ${TEST_FILE})

    if(${TEST_NAME} STREQUAL "CMyWindow")
        add_executable(${TEST_NAME} ${TEST_FILE} "../res/resource.rc")

        target_compile_definitions(${TEST_NAME} PRIVATE _AFXDLL __TEST)
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
    ) #? CMake 3.10+

    list(APPEND TEST_NAMES "${TEST_NAME}")
endforeach()

add_custom_target(unit_tests DEPENDS ${TEST_NAMES})

message(STATUS "Created `unit_tests` build target that will build all tests found in ${CMAKE_CURRENT_SOURCE_DIR} (${TEST_NAMES})")
