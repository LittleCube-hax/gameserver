cmake_minimum_required(VERSION 3.11)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

project(gameserver)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "$\{ORIGIN\}")

add_executable(${PROJECT_NAME})

set(SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.c
    ${PROJECT_SOURCE_DIR}/src/client/client.c
    ${PROJECT_SOURCE_DIR}/src/client/incoming.c
    ${PROJECT_SOURCE_DIR}/src/client/outgoing.c
    ${PROJECT_SOURCE_DIR}/src/config.c
    ${PROJECT_SOURCE_DIR}/src/heap.c
    ${PROJECT_SOURCE_DIR}/src/swap_vector.c
    ${PROJECT_SOURCE_DIR}/src/utils.c
    
    ${PROJECT_SOURCE_DIR}/lib/o1heap/o1heap/o1heap.c
    ${PROJECT_SOURCE_DIR}/lib/tomlc17/src/tomlc17.c
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_compile_options(${PROJECT_NAME} PRIVATE -static)

# frick u ninja
if (${CMAKE_GENERATOR} MATCHES "Ninja")
set(CONFIG_DIR .)
else()
set(CONFIG_DIR $<CONFIG>)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/client
    ${PROJECT_SOURCE_DIR}/lib/o1heap/o1heap
    ${PROJECT_SOURCE_DIR}/lib/tomlc17/src
)