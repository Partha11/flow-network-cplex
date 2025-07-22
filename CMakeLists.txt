cmake_minimum_required(VERSION 3.10)
project(CPlexMathModeling LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
# Debug mode
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# Find CPLEX
if(NOT DEFINED ENV{CPLEX_HOME})
    message(FATAL_ERROR "CPLEX_HOME environment variable is not set. Please set it to your CPLEX installation directory.")
endif()
set(CPLEX_HOME "$ENV{CPLEX_HOME}")

# Verify CPLEX installation
if(NOT EXISTS "${CPLEX_HOME}")
    message(FATAL_ERROR "CPLEX_HOME directory does not exist: ${CPLEX_HOME}")
endif()

# Detect system architecture and set library path accordingly
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64-bit system
    set(CPLEX_ARCH "x86-64_linux")
else()
    # 32-bit system
    set(CPLEX_ARCH "x86_linux")
endif()

# Set CPLEX library directory - try both static_pic and shared versions
set(CPLEX_LIB_DIR_STATIC "${CPLEX_HOME}/lib/${CPLEX_ARCH}/static_pic")
set(CPLEX_LIB_DIR_SHARED "${CPLEX_HOME}/lib/${CPLEX_ARCH}")

# Check which library directory exists
if(EXISTS "${CPLEX_LIB_DIR_STATIC}")
    set(CPLEX_LIB_DIR "${CPLEX_LIB_DIR_STATIC}")
    message(STATUS "Using CPLEX static_pic libraries from: ${CPLEX_LIB_DIR}")
elseif(EXISTS "${CPLEX_LIB_DIR_SHARED}")
    set(CPLEX_LIB_DIR "${CPLEX_LIB_DIR_SHARED}")
    message(STATUS "Using CPLEX shared libraries from: ${CPLEX_LIB_DIR}")
else()
    message(FATAL_ERROR "Could not find CPLEX libraries in expected directories:
    ${CPLEX_LIB_DIR_STATIC}
    ${CPLEX_LIB_DIR_SHARED}")
endif()

# Set Concert library directory
set(CONCERT_HOME "${CPLEX_HOME}/../concert")
set(CONCERT_LIB_DIR "${CONCERT_HOME}/lib/${CPLEX_ARCH}/static_pic")

if(NOT EXISTS "${CONCERT_LIB_DIR}")
    set(CONCERT_LIB_DIR "${CONCERT_HOME}/lib/${CPLEX_ARCH}")
endif()

if(NOT EXISTS "${CONCERT_LIB_DIR}")
    message(FATAL_ERROR "Could not find Concert libraries in: ${CONCERT_LIB_DIR}")
endif()

# Print paths for debugging
message(STATUS "CPLEX_HOME: ${CPLEX_HOME}")
message(STATUS "CPLEX include directory: ${CPLEX_HOME}/include")
message(STATUS "Concert include directory: ${CONCERT_HOME}/include")
message(STATUS "CPLEX library directory: ${CPLEX_LIB_DIR}")
message(STATUS "Concert library directory: ${CONCERT_LIB_DIR}")

# Find source files
file(GLOB SOURCE_FILES "src/*.cpp")

# Create executable
add_executable(cplex_app ${SOURCE_FILES})

# Set include directories
target_include_directories(cplex_app PRIVATE 
    "${CPLEX_HOME}/include"
    "${CONCERT_HOME}/include"
    "include"
)

# Find CPLEX libraries with full paths
find_library(CPLEX_LIBRARY
    NAMES cplex
    PATHS "${CPLEX_LIB_DIR}"
    NO_DEFAULT_PATH
)

find_library(ILOCPLEX_LIBRARY
    NAMES ilocplex
    PATHS "${CPLEX_LIB_DIR}"
    NO_DEFAULT_PATH
)

find_library(CONCERT_LIBRARY
    NAMES concert
    PATHS "${CONCERT_LIB_DIR}"
    NO_DEFAULT_PATH
)

# Check if libraries were found
if(NOT CPLEX_LIBRARY)
    message(FATAL_ERROR "CPLEX library not found in ${CPLEX_LIB_DIR}")
endif()

if(NOT ILOCPLEX_LIBRARY)
    message(FATAL_ERROR "ILOCPLEX library not found in ${CPLEX_LIB_DIR}")
endif()

if(NOT CONCERT_LIBRARY)
    message(FATAL_ERROR "Concert library not found in ${CONCERT_LIB_DIR}")
endif()

message(STATUS "Found CPLEX library: ${CPLEX_LIBRARY}")
message(STATUS "Found ILOCPLEX library: ${ILOCPLEX_LIBRARY}")
message(STATUS "Found Concert library: ${CONCERT_LIBRARY}")

# Link libraries
target_link_libraries(cplex_app PRIVATE
    ${ILOCPLEX_LIBRARY}
    ${CONCERT_LIBRARY}
    ${CPLEX_LIBRARY}
    pthread
    dl
    m
)

# Set compiler definitions for CPLEX
target_compile_definitions(cplex_app PRIVATE
    IL_STD
)

# Output directory
set_target_properties(cplex_app PROPERTIES 
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Export compile commands for VSCode IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)