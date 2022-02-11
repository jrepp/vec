cmake_minimum_required(VERSION 3.13)
project(vec
        VERSION 0.3.0
        LANGUAGES C)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(VEC_ENABLE_CODE_COVERAGE "Enable code coverage for tests" OFF)

macro(configure_compiler TARGET_NAME)
    target_compile_features(${TARGET_NAME} PUBLIC c_std_99)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wunused-value)
    endif()
endmacro()

#
# vec library config
#
set(VEC_SOURCES src/vec.c src/vec.h src/vec_config_default.h)
add_library(vec STATIC ${VEC_SOURCES})
configure_compiler(vec)

#
# vec basic test suite config
#
set(VEC_TEST_SOURCES
        test/test_main.c
        test/test_mem.c
        test/test_stats.c
        test/test_vec_custom.c
        test/test_vec_fixed.c
        test/test_vec_ops.c
        test/test_mem.c
        test/test_vec_mem_failures.c
        test/test_help.h
        test/vec_config_test.h)
add_executable(test_vec ${VEC_TEST_SOURCES} ${VEC_SOURCES})
configure_compiler(test_vec)
target_include_directories(test_vec PRIVATE src/ test/)
target_compile_definitions(test_vec PRIVATE VEC_CONFIG_H="vec_config_test.h")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Debug' as none was specified.")
    set(CMAKE_BUILD_TYPE "Debug" CACHE
            STRING "Choose the type of build." FORCE)
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
            "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

include(cmake/CodeCoverage.cmake)
if (VEC_ENABLE_CODE_COVERAGE AND GCOVR_PATH)
    message(STATUS "Code coverage support enabled")
    append_coverage_compiler_flags()
    setup_target_for_coverage_gcovr_xml(
            NAME coverage
            EXECUTABLE test_vec
            EXCLUDE "${PROJECT_SOURCE_DIR}/test/*")
else()
    message(STATUS "Code coverage support disabled")
endif()




# configure ctest
enable_testing()
add_test(NAME basics 
         COMMAND test_vec)



