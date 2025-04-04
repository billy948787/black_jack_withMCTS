cmake_minimum_required(VERSION 3.30)

project(blackjack)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_executable(blackjack ${SOURCES} "src/main.cpp")

file(GLOB TEST_SOURCES "test/*.cpp")

add_executable(test_main ${TEST_SOURCES} ${SOURCES})

add_custom_target(
    profile
    COMMAND xcrun xctrace record --template "Time Profiler" --output "${CMAKE_CURRENT_BINARY_DIR}/blackjack.trace" --launch -- ${CMAKE_CURRENT_BINARY_DIR}/blackjack
    DEPENDS blackjack
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Profiling with Instruments"
)

if(TEST_SOURCES)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    target_link_libraries(test_main gtest_main gtest gmock gmock_main)

    add_test(NAME test COMMAND test_main)
endif()
