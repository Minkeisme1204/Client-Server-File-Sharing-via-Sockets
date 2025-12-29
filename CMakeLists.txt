cmake_minimum_required(VERSION 3.16)

project(FileTransferCore
    VERSION 1.0.0
    LANGUAGES CXX
)

find_package(Threads REQUIRED)

# =========================
# C++ standard
# =========================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =========================
# Collect all source files
# =========================
file(GLOB_RECURSE CORE_SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/core/*.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/*.cpp
)

# =========================
# Create single library
# =========================
add_library(filetransfer STATIC
    ${CORE_SOURCES}
)

# =========================
# Include directories
# =========================
target_include_directories(filetransfer
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/core/Client
        ${PROJECT_SOURCE_DIR}/include/core/Server
)

target_link_libraries(filetransfer
    PUBLIC
        pthread
)

# =========================
# Compile options (optional)
# =========================
target_compile_options(filetransfer
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
)

# =========================
# (Optional) Position Independent Code
# useful if later link into shared lib
# =========================
set_target_properties(filetransfer
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

# =========================
# Build test executables
# =========================
add_executable(server_test
    ${PROJECT_SOURCE_DIR}/tests/server_test.cpp
)

target_link_libraries(server_test
    PRIVATE
        filetransfer
)

add_executable(client_test
    ${PROJECT_SOURCE_DIR}/tests/client_test.cpp
)

target_link_libraries(client_test
    PRIVATE
        filetransfer
)

add_executable(simp_client
    ${PROJECT_SOURCE_DIR}/tests/simple_client.cpp
)

add_executable(simp_server
    ${PROJECT_SOURCE_DIR}/tests/simple_server.cpp
)

add_executable(bulk_upload_client
    ${PROJECT_SOURCE_DIR}/tests/bulk_upload_client.cpp
)

target_link_libraries(bulk_upload_client
    PRIVATE
        filetransfer
)
