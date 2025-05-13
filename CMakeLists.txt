# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project Name and Language
project(SFMLBlobGame LANGUAGES CXX)

# Set C++ Standard (SFML requires at least C++11, C++17 is recommended)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: Enforce standard C++ without GNU extensions

# --- Configure SFML Build (Optional but Recommended) ---
# You can disable parts of SFML you don't need to speed up compilation slightly.
# By default, SFML builds Audio and Network too.
# set(SFML_BUILD_AUDIO OFF CACHE BOOL "Disable building sfml-audio")
# set(SFML_BUILD_NETWORK OFF CACHE BOOL "Disable building sfml-network")
# set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "Disable building SFML examples") # Recommended
# set(SFML_BUILD_TESTING OFF CACHE BOOL "Disable building SFML tests")    # Recommended

# Disable UDev requirement for Joystick support (will disable joystick support)
set(SFML_USE_JOYSTICK_UDEV OFF CACHE BOOL "Disable using UDev for Joystick support")

# --- Add SFML Source Directory ---
# This command tells CMake to look into dep/sfml and run its CMakeLists.txt.
# This will define the SFML library targets (like sfml-graphics, sfml-window, etc.)
# IMPORTANT: SFML needs to be added *before* your executable target that uses it.
# Use EXCLUDE_FROM_ALL if you don't want SFML targets to appear in IDEs by default
# or built when running a generic 'build all' command (they will still be built
# as dependencies of your main executable).
add_subdirectory(dep/sfml EXCLUDE_FROM_ALL)

# --- Define Your Game Executable ---
# List your source files here
add_executable(sfml_blob main.cpp Systems.cpp Components.cpp)

# --- Link Your Executable Against SFML Libraries ---
# CMake knows about the 'sfml-graphics', 'sfml-window', and 'sfml-system' targets
# because we ran add_subdirectory(dep/sfml).
# Linking against these targets automatically handles include directories
# and linking dependencies (like OpenGL, freetype, etc.) based on SFML's own CMake setup.
# Use PRIVATE since these are implementation details of your executable.
target_link_libraries(sfml_blob PRIVATE sfml-graphics sfml-window sfml-system)

# --- Platform Specific Adjustments (Often handled by SFML's CMake now) ---
# In modern SFML with CMake, explicit handling for things like frameworks on macOS
# or specific libraries on Linux is usually done internally by SFML's CMakeLists.txt
# when you link via target_link_libraries.

# Example: If you were building SFML statically (not the default, requires specific SFML CMake options)
# You might need this on Windows:
# if(WIN32 AND SFML_STATIC_LIBRARIES)
#   target_compile_definitions(sfml_blob PRIVATE SFML_STATIC)
# endif()

# --- Installation (Optional) ---
# If you want to create an install target (e.g., for packaging)
# include(GNUInstallDirs)
# install(TARGETS sfml_blob DESTINATION ${CMAKE_INSTALL_BINDIR})
# You might also need to handle installing SFML DLLs on Windows if building dynamically.
# The 'deploy sfml' script or CMake install components in SFML might help here.
