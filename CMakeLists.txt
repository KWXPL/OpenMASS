cmake_minimum_required(VERSION 3.25)

project(OpenMASS VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(
    gps_console
    gps/src/main.cpp
    gps/src/at_command_client.cpp
    gps/src/gnss_info.cpp
    gps/src/gnss_proto.cpp
    gps/src/serial_port.cpp
)

target_include_directories(gps_console PRIVATE gps/headers)

target_compile_options(
    gps_console
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
)

include(CTest)

if(BUILD_TESTING)
    add_executable(
        gnss_proto_test
        gps/tests/gnss_proto_test.cpp
        gps/src/gnss_info.cpp
        gps/src/gnss_proto.cpp
    )
    target_include_directories(gnss_proto_test PRIVATE gps/headers)
    target_compile_options(
        gnss_proto_test
        PRIVATE
            $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall -Wextra -Wpedantic>
            $<$<CXX_COMPILER_ID:Clang,GNU>:-UNDEBUG>
    )
    add_test(NAME gnss_proto_test COMMAND gnss_proto_test)
endif()
