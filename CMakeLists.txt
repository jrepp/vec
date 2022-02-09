cmake_minimum_required(VERSION 3.21)
project(vec)

macro(configure_compiler TARGET_NAME)
    target_compile_features(${TARGET_NAME} PUBLIC c_std_99)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endmacro()

# vec library
add_library(vec STATIC src/vec.c)
configure_compiler(vec)

# vec test suite
add_executable(test_vec test/test_vec.c)
configure_compiler(test_vec)
target_include_directories(test_vec PRIVATE src/)
target_link_libraries(test_vec vec)


