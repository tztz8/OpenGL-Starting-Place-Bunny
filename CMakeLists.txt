cmake_minimum_required(VERSION 3.20)
project(OpenGL-Starting-Place-Bunny)

# Set the version of CPP we are using
set(CMAKE_CXX_STANDARD 17)

# Set things static
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC) # microsoft visual code requires this package when working with opengl
    find_package(OpenGL REQUIRED)
endif()

if(MSVC) # microsoft visual code requires this Preprocessor Definitions for when using Modified GLM (obj file loaders)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (WIN32 AND NOT MSVC) # most compiler's other then microsoft visual code on windows need cmake to use GLM package
    find_package(GLM REQUIRED)
    message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
endif()

if (UNIX AND NOT APPLE) # on linux glm is in /usr/include
    include_directories(. /usr/include)
endif()

# add glfw lib (note glfw is a submodule of this git repo)
if (NOT MSVC)
    find_package(glfw3 QUIET)
    if (NOT glfw3_FOUND)
        add_subdirectory(exernalLibraries/glfw)
        link_libraries(glfw ${GLFW_LIBRARIES})
    endif()
else()
    add_subdirectory(exernalLibraries/glfw)
    link_libraries(glfw ${GLFW_LIBRARIES})
endif()

# Add Dev Image Lib
find_package(DevIL REQUIRED)
message(STATUS "DevIL included at ${IL_INCLUDE_DIR}")

# basicGLFW project
add_subdirectory(basicGLFW)

# basicFreeGLUT project
add_subdirectory(basicFreeGLUT)
