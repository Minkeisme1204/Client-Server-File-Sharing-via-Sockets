cmake_minimum_required(VERSION 3.10)

# Set Qt5 CMake policy
cmake_policy(SET CMP0071 NEW)

# Find Qt5
find_package(Qt5 REQUIRED COMPONENTS Widgets Network)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/core/Client)
include_directories(${CMAKE_SOURCE_DIR}/include/core/Server)

# Client GUI Application
add_executable(client_gui
    client/main.cpp
    client/ClientWindow.cpp
    client/ClientWindow.h
)

target_link_libraries(client_gui
    Qt5::Widgets
    Qt5::Network
    filetransfer  # Link to the existing library
    pthread
)

# Server GUI Application
add_executable(server_gui
    server/main.cpp
    server/ServerWindow.cpp
    server/ServerWindow.h
    server/ServerThread_impl.cpp
)

target_link_libraries(server_gui
    Qt5::Widgets
    Qt5::Network
    filetransfer  # Link to the existing library
    pthread
)

# Set output directory
set_target_properties(client_gui server_gui
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
