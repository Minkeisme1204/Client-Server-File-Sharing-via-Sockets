cmake_minimum_required(VERSION 3.16)

project(FileTransferGUI VERSION 1.0.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 or Qt5
find_package(Qt6 COMPONENTS Core Widgets QUIET)
if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

message(STATUS "Using Qt${QT_VERSION_MAJOR}")

# Include parent project's library
set(PARENT_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../build)
set(PARENT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)

# Find the filetransfer library
find_library(FILETRANSFER_LIB
    NAMES filetransfer libfiletransfer.a
    PATHS ${PARENT_BUILD_DIR}
    NO_DEFAULT_PATH
    REQUIRED
)

message(STATUS "Found filetransfer library: ${FILETRANSFER_LIB}")

# Client GUI Application
add_executable(client_gui
    client/main.cpp
    client/ClientWindow.cpp
    client/ClientWindow.h
)

target_include_directories(client_gui PRIVATE
    ${PARENT_INCLUDE_DIR}
    ${PARENT_INCLUDE_DIR}/core/Client
    ${PARENT_INCLUDE_DIR}/core/Server
)

target_link_libraries(client_gui PRIVATE
    ${FILETRANSFER_LIB}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    pthread
)

# Server GUI Application
add_executable(server_gui
    server/main.cpp
    server/ServerWindow.cpp
    server/ServerWindow.h
)

target_include_directories(server_gui PRIVATE
    ${PARENT_INCLUDE_DIR}
    ${PARENT_INCLUDE_DIR}/core/Client
    ${PARENT_INCLUDE_DIR}/core/Server
)

target_link_libraries(server_gui PRIVATE
    ${FILETRANSFER_LIB}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    pthread
)

# Set output directory
set_target_properties(client_gui server_gui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
