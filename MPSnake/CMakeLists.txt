﻿# CMakeList.txt : CMake project for MPSnake, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML COMPONENTS graphics audio REQUIRED)

# Add source to this project's executable.
add_executable (MPSnakeClient "Client.cpp" "Client.hpp" "enet.h" "Serialization.hpp" "Message.hpp" "Game.hpp")
add_executable (MPSnakeServer "Server.cpp" "Server.hpp" "enet.h" "Serialization.hpp" "Message.hpp" "Game.hpp")

# TODO: Add tests and install targets if needed.
target_link_libraries(MPSnakeClient sfml-graphics sfml-audio sfml-system)
target_link_libraries(MPSnakeServer sfml-system)