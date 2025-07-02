cmake_minimum_required(VERSION 3.22)

project(Redbrouk)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
-fmacro-prefix-map=${CMAKE_SOURCE_DIR}=REDBROUK
)
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
#add_compile_options(-g -O0 -fsanitize=address)
#add_link_options(-fsanitize=address)

add_subdirectory(src)
add_subdirectory(test)
