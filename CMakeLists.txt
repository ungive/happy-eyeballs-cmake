cmake_minimum_required(VERSION 3.13)
project(happy_eyeballs LANGUAGES C)

option(HAPPY_EYEBALLS_BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    add_library(metadata_msvc_flags INTERFACE)
    target_compile_options(metadata_msvc_flags INTERFACE /utf-8 /EHsc)
endif()

add_library(happy_eyeballs STATIC src/rfc6555.c)

target_include_directories(happy_eyeballs PUBLIC include)
target_include_directories(happy_eyeballs PRIVATE src)

if(WIN32)
    target_link_libraries(happy_eyeballs PRIVATE Ws2_32)
    target_link_libraries(happy_eyeballs PRIVATE metadata_msvc_flags)
endif()

if(HAPPY_EYEBALLS_BUILD_TESTS)
    add_executable(rfc6555_test test/rfc6555_test.c)
    target_link_libraries(rfc6555_test happy_eyeballs)

    add_executable(gai_test test/main.c test/gai.c)
    target_link_libraries(gai_test happy_eyeballs)

    add_executable(happy_test test/main.c test/happy.c)
    target_link_libraries(happy_test happy_eyeballs)
endif()
