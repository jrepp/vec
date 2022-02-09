cmake_minimum_required(VERSION 3.13)
project(vec
        VERSION 0.3.0
        LANGUAGES C)

set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

macro(configure_compiler TARGET_NAME)
    target_compile_features(${TARGET_NAME} PUBLIC c_std_99)
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
    else()
        target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wunused-value)
    endif()
endmacro()

# vec library
add_library(vec STATIC src/vec.c src/vec.h)
configure_compiler(vec)

# vec basic test suite
add_executable(test_vec test/test_vec.c)
configure_compiler(test_vec)
target_include_directories(test_vec PRIVATE src/)
target_link_libraries(test_vec vec)

# configure ctest
enable_testing()
add_test(NAME basics 
         COMMAND test_vec)



