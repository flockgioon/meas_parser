cmake_minimum_required(VERSION 3.25)
project(meas_parser LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(project_warning INTERFACE)
target_compile_options(project_warning
    INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
        -Wshadow
        -Wconversion
        -Wmissing-prototypes
)

# lib: core
add_library(core STATIC
    src/big_endian.c
    src/crc.c
    src/mp_frame.c
    src/mp_parser.c
    src/mp_decode.c
    src/mp_log.c
)
target_include_directories(core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(core
    PRIVATE
        c_std_11
)
target_link_libraries(core
    PRIVATE
        project_warning
)


# executable: meas_parser
add_executable(meas_parser
    src/main.c
)
target_link_libraries(meas_parser
    PRIVATE
        project_warning
        core
)
target_compile_features(meas_parser
    PRIVATE
        c_std_11
)

# executable: simu
add_executable(simu
    tools/instrument_simulator.c
)
target_link_libraries(simu
    PRIVATE
        project_warning
        core
)
target_compile_features(simu
    PRIVATE
        c_std_11
)

# test
include(CTest)

function(add_test_helper target_name)
    add_executable(${target_name}
        tests/${target_name}.c
    )
    target_link_libraries(${target_name}
        PRIVATE
            project_warning
            core
    )
    target_compile_features(${target_name}
        PRIVATE
            c_std_11
    )

    add_test(
        NAME ${target_name}
        COMMAND ${target_name}
    )
endfunction()

if (BUILD_TESTING)
    add_test_helper(test_big_endian)
    add_test_helper(test_crc)
    add_test_helper(test_mp_frame)
    add_test_helper(test_mp_parser)
    add_test_helper(test_mp_decode)
endif()
