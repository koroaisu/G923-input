cmake_minimum_required(VERSION 3.21)
project(G923Input LANGUAGES CXX)

add_executable(g923-input src/main.cpp)
set_target_properties(g923-input PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

if(MSVC)
    target_compile_options(g923-input PRIVATE /W4 /utf-8)
else()
    target_compile_options(g923-input PRIVATE
        -Wall -Wextra -finput-charset=UTF-8 -fexec-charset=UTF-8
    )
endif()

target_link_libraries(g923-input PRIVATE dinput8 dxguid uuid)
