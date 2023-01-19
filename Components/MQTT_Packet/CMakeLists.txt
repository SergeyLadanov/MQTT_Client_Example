cmake_minimum_required(VERSION 3.10)

# Get current path
set(path ${CMAKE_CURRENT_SOURCE_DIR})
# Put directory name to COMPONENT_NAME variable
get_filename_component(COMPONENT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
# Set component name
project(${COMPONENT_NAME})

# Source files
FILE(
    GLOB_RECURSE
    SOURCE_LIB 
    "*.cpp"
    "*.c"
    )


# Add includes
include_directories(
"./"
)

# Creating static library
add_library(${COMPONENT_NAME} STATIC ${SOURCE_LIB})

