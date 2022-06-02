# set a name for the executable (Expecting the name of the program to be the same as folder the code is in)
set(executable_name basicFreeGLUT)

# add the executable (also tell it all the src files)
add_executable(${executable_name}
        res/icon/programIcon.rc # for in windows add a icon to the exe file and for freeglut on windows to add a icon to window
        GLM.cpp GLM.h # The Modified GLM lib
        OpenGLHelperMethods.h # OpenGl Helper Methods like load shaders
        main.cpp main.h# main (start of our executable)
        Cube.cpp Cube.h) # any other files (code files and not shaders) for our project

if (UNIX AND NOT APPLE) # Linux
    target_link_libraries(${executable_name} ${IL_LIBRARIES} -lglut -lGL -lGLEW -lGLU)
    message(NOTICE "Change the Working Directory for ${executable_name} to ${CMAKE_SOURCE_DIR}/${executable_name}")
elseif (MSVC) # Windows using microsoft visual code
    target_link_libraries(${executable_name} ${IL_LIBRARIES} glew32 -lOpenGL32 -lfreeGLUT)
    # using when converting the project from cmake to microsoft visual code project
    set_target_properties(${executable_name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${executable_name}")
    # Because microsoft visual code can't change working directory when using cmake these lines copy the files to the build directory so it like it in working directory
    file(COPY shader.frag DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY shader.vert DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY bunny.obj DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY bunny.obj.mtl DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(COPY res/3D_pattern_58/pattern_290/diffuse.tga DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/res/3D_pattern_58/pattern_290/)
    file(COPY res/3D_pattern_58/pattern_292/diffuse.tga DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/res/3D_pattern_58/pattern_292/)
    # Let the user know where the working directory for the program goes
    message(STATUS "${executable_name} Working Directory at ${CMAKE_SOURCE_DIR}/${executable_name}")
else() # Windows
    target_link_libraries(${executable_name} ${IL_LIBRARIES} glm glew32 -lOpenGL32 -lfreeGLUT)
    message(NOTICE "Change the Working Directory for ${executable_name} to ${CMAKE_SOURCE_DIR}/${executable_name}")
endif()

# NOTE I do not have a mac to tests with
# external include folders
target_include_directories(${executable_name} PRIVATE ${IL_INCLUDE_DIR})