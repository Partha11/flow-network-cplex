cmake_minimum_required(VERSION 3.10)
project(CPlexMathModeling LANGUAGES CXX)

# find CPLEX
if(NOT DEFINED ENV{CPLEX_HOME})
    message(FATAL_ERROR "CPLEX_HOME environment variable is not set. Please set it to your CPLEX installation directory (e.g., /opt/ibm/ILOG/CPLEX_Studio2211/cplex).")
endif()
set(CPLEX_HOME "$ENV{CPLEX_HOME}")

file(GLOB SOURCE_FILES "src/*.cpp")

# Executables
add_executable(cplex_app ${SOURCE_FILES})

# CPLEX header files
include_directories(${CPLEX_HOME}/include)

# Custom header files
target_include_directories(cplex_app PRIVATE include)

# Link CPLEX libraries
set(CPLEX_LIB_DIR "${CPLEX_HOME}/lib/x86-64_linux/c++11_shared")

# Linker search path
link_directories("${CPLEX_LIB_DIR}")

# Link specific CPLEX libraries
target_link_libraries(cplex_app PRIVATE
    ilocplex
    cplex
    concert
)

# Output directory
set_target_properties(cplex_app PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")