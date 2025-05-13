# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(SFMLBlobGame LANGUAGES CXX)

# Set C++ Standard (SFML requires at least C++11, C++17 is recommended)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: Enforce standard C++ without GNU extensions

# --- Configure SFML Build (Optional but Recommended) ---
# set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable building sfml-audio")
# set(SFML_BUILD_NETWORK OFF CACHE BOOL "Disable building sfml-network")
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable building SFML examples") # Recommended
set(SFML_BUILD_TESTING OFF CACHE BOOL "Disable building SFML tests")    # Recommended
set(SFML_USE_JOYSTICK_UDEV OFF CACHE BOOL "Disable using UDev for Joystick support")

# --- Add SFML Source Directory ---
add_subdirectory(dep/sfml EXCLUDE_FROM_ALL)

# ---- Add Box2d Source Directory ---
# This builds Box2D and defines the 'Box2D' target (usually)
add_subdirectory(dep/box2d)


# --- Define Your Game Executable ---
# Add src/game_object.cpp to the list of sources
add_executable(sfml_blob main.cpp src/game_object.cpp)

# --- Add Include Directory ---
# This tells CMake where to find your new header files (constants.hpp, utils.hpp, game_object.hpp)
target_include_directories(sfml_blob PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- Link Your Executable Against SFML and Box2D Libraries ---
# Link against SFML modules AND the Box2D target built by add_subdirectory
target_link_libraries(sfml_blob PRIVATE sfml-graphics sfml-window sfml-system box2d)