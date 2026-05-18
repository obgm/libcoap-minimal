cmake_minimum_required(VERSION 3.20)

project(MY_PROJECT VERSION 0.1.0 LANGUAGES CXX C)
set(TARGET myproject)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET} client.cc common.cc)
install(TARGETS ${TARGET} DESTINATION bin)

target_include_directories(${TARGET}
    PUBLIC ${PROJECT_BINARY_DIR}
)

target_link_libraries(${TARGET}
    PRIVATE coap-3
)
