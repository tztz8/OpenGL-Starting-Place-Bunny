#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
/**
 * Main
 */

//          --- Libraries ---

// Normal Lib
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <cctype>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// image lib
#include <IL/il.h>
//#include <IL/ilu.h>
//#include <IL/ilut.h>

// Modified GLM (obj file loaders)
#include "GLM.h"

// image lib (glfw)
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Math Lib
#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

#include "main.h"
#include "OpenGLHelperMethods.h"

// h file of this assignment
#include "Cube.h"

//          --- Filled's ---

// glfw window id
GLFWwindow* window;

// Bool to know when to exit
bool exitWindowFlag = false;

/**
 * if the program is set to fullscreen
 */
bool isFullScreen = false; // do not change // look at main to make it full screen

// initial screen size
int screenWidth = 512, screenHeight = 512;

// Current screen size
GLint glScreenWidth, glScreenHeight;

// flag to know when screen size changes
bool freeGLUTSizeUpdate;

// title info
#define TITLE_LENGTH 100

// devIL is setup flag
bool devILIsSetup = false; // DO NOT CHANGE

// Window GL variables
/**
 * Aspect ratio <br>
 * Proportion between the width and the height of the window
 */
GLfloat aspect = float(screenWidth) / float(screenHeight);

// Booleans for current state
/**
 * Flag if to stop the rotate of the camera around the object
 */
bool stop_rotate = true;
/**
 * Flag to show the lines (not fill the trinalges)
 */
bool show_line = false;
/**
 * Flag to show the lines with GL_CULL_FACE (not fill the trinalges)
 */
bool show_line_new = false;
/**
 * Move the camera to look from above and change rotate to rotate the up vector
 */
bool top_view_flag = false;

// GL loc
/**
 * The location of the model matrix in the shader
 */
GLint matrix_loc;
/**
 * The location of the projection matrix in the shader
 */
GLint projection_matrix_loc;
/**
 * The location of the view (Camera) matrix in the shader
 */
GLint view_matrix_loc;

// shader program
/**
 * The handle of the shader program object.
 */
GLuint program;

// Matrix's
/**
 * Camera matrix <br>
 * Use glm::lookAt to make
 */
glm::mat4 view_matrix(1.0f);
/**
 * 3d to 2d Matrix <br>
 * Normally using glm::perspective to make
 */
glm::mat4 projection_matrix(1.0f);
/**
 * matrix to apply to things being dawn <br>
 * Often use at less one of these <br>
 *     - glm::scale <br>
 *     - glm::translate <br>
 *     - glm::rotate <br>
 */
glm::mat4 model_matrix(1.0f);

// Add light components
/**
 * Vector of where the light position in 3d world
 */
glm::vec4 light_position(10.0, 6.0, 8.0, 1.0);

/**
 * Vector of where the light position in 3d canvas from using view (camera) matrix and 3d world position
 */
glm::vec4 light_position_camera;

// uniform indices of light
/**
 * The location of the light position in the shader
 */
GLint light_position_loc;

// Angle
/**
 * Angle used for rotating the view (camera)
 */
GLfloat rotateAngle = 0.0f;

/* Class GLM object*/
GLMmodel* objmodel = nullptr;
GLuint objmodel_vao, objmodel_vbo, objmodel_ebo;
GLuint imageTexID;

GLuint image2TexID;

//          --- Methods ---

/**
 * Set all the gl uniform for currentProgram
 * @param shaderProgram to set as the current shader program being used
 */
void setUniformLocations(GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    view_matrix_loc = glGetUniformLocation(shaderProgram, "view_matrix");
    matrix_loc = glGetUniformLocation(shaderProgram, "model_matrix");
    projection_matrix_loc = glGetUniformLocation(shaderProgram, "projection_matrix");
    light_position_loc = glGetUniformLocation(shaderProgram, "LightPosition");
    glUniform1i(glGetUniformLocation(shaderProgram, "Tex1"), 0);
}

/**
 * Called set setup open gl things (for example making the models)
 */
void Initialize(){

    // Get the bunny model
    if (!objmodel) {
        std::string filename = "bunny.obj";
        objmodel = glmReadOBJ(filename.c_str());
        if (!objmodel) {
            fprintf(stderr, "ERROR: OBJ file does not exist \n");
            exit(EXIT_FAILURE);
        } else {
            fprintf(stdout, "Info: Initialize: OBJ file \"%s\" loaded\n", filename.c_str());
        }
        unitizeModel(objmodel->vertices, objmodel->numvertices);
        updateVertexNormals(objmodel->vertices, objmodel->normals,
                            objmodel->indices, objmodel->numnormals, objmodel->numindices);
    }

    // Create the program for rendering the model
//    program = initShaders("shader.vert", "shader.frag");
    // or
    const char* s[] = {"shader.vert", "shader.frag"};
    program = initShaders(s, 2);

    // Check if making the shader work or not // This is not in FreeGLUT as does need an exit flag
    if (exitWindowFlag) {
        return;
    }

    // attribute indices
    model_matrix = glm::mat4(1.0f);

    // Use the shader program
    setUniformLocations(program);

    // load textures
    imageTexID = loadTexture("res/3D_pattern_58/pattern_290/diffuse.tga");
    image2TexID = loadTexture("res/3D_pattern_58/pattern_292/diffuse.tga");

    // Set Clear Color (background color)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Setup bunny in GPU
    if (objmodel) {
        // VertexArray
        glGenVertexArrays(1, &objmodel_vao);
        glBindVertexArray(objmodel_vao);
        GLuint offset = 0;
        glGenBuffers(1, &objmodel_vbo);

        glBindBuffer(GL_ARRAY_BUFFER, objmodel_vbo);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(
                             sizeof(glm::vec3) * (objmodel->numvertices) + // Vertex position
                             sizeof(glm::vec3) * (objmodel->numvertices) + // Vertex normal
                             sizeof(glm::vec2) * (objmodel->numvertices)), // Vertex textures
                     nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, offset,
                        static_cast<GLsizeiptr>(sizeof(glm::vec3) * (objmodel->numvertices)), // Vertex position
                        objmodel->vertices);
        glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);  // Vertex position

        offset += sizeof(glm::vec3) * (objmodel->numvertices); // Vertex position
        glBufferSubData(GL_ARRAY_BUFFER, offset,
                        static_cast<GLsizeiptr>(sizeof(glm::vec3) * (objmodel->numvertices)), // Vertex normal
                        objmodel->normals);
        glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(glm::vec3) * (objmodel->numvertices)));
        glEnableVertexAttribArray(1);  // Vertex normal

        offset += sizeof(glm::vec3) * (objmodel->numvertices); // Vertex normal
        glBufferSubData(GL_ARRAY_BUFFER, offset,
                        static_cast<GLsizeiptr>(sizeof(glm::vec2) * (objmodel->numvertices)), // Vertex textures
                        objmodel->textures);
        glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(glm::vec3) * (objmodel->numvertices + objmodel->numvertices)));
        glEnableVertexAttribArray(2);  // Vertex textures

        glGenBuffers(1, &objmodel_ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, objmodel_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(sizeof(GLuint) * (objmodel->numindices)), // Indices
                     objmodel->indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    // setup Cube
    createCube();
}

/**
 * Called for every frame to draw on the screen
 */
void Display() {
    // Clear
    if (freeGLUTSizeUpdate) {
        glViewport(0, 0, glScreenWidth, glScreenHeight);
        freeGLUTSizeUpdate = false;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // Show Lines
    // Tell GL to use GL_CULL_FACE
    if (show_line_new) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    } else {
        glDisable(GL_CULL_FACE);
    }
    // Tell to fill or use Lines (not to fill) for the triangles
    if (show_line || show_line_new) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Set Point Size
    glPointSize(10);

    // Set view matrix
    float rotateAngleRadians = glm::radians(rotateAngle);
    if (top_view_flag) { // Top View
        view_matrix = glm::lookAt(
                glm::vec3(0.0, 8.0f, 0.0), // camera is at the top
                glm::vec3(0, 0, 0), // look at the center
                glm::vec3(
                        sinf(rotateAngleRadians),
                        0.0f,
                        cosf(rotateAngleRadians)
                ) // rotating the camera
        );
    } else { // Normal View
        view_matrix = glm::lookAt(
                glm::vec3(
                        8.0f * sinf(rotateAngleRadians),
                        3.0f,
                        8.0f * cosf(rotateAngleRadians)
                ), // Moving around the center in a Center
                glm::vec3(0, 0, 0), // look at the center
                glm::vec3(0, 1, 0) // keeping the camera up
        );
    }
    // Let opengl know about the change
    glUniformMatrix4fv(view_matrix_loc, 1, GL_FALSE, (GLfloat*)&view_matrix[0]);

    // update light_position_camera base off on both light position and view matrix
    light_position_camera = view_matrix * light_position;
    glUniform4fv(light_position_loc, 1, &light_position_camera[0]);

    // update projection matrix (useful when the window resize)
    projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.3f, 100.0f);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, (GLfloat*)&projection_matrix[0]);

    // ---- Draw things ----

    // Draw Bunny
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imageTexID);
    glBindVertexArray(objmodel_vao);
    model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
    glDrawElements(GL_TRIANGLES,objmodel->numindices, GL_UNSIGNED_INT, nullptr);

    // Draw Cube
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image2TexID);
    model_matrix =
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f));
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
    drawCube();

    // ---- End of Draw things ----
    glFlush();
}

// ------------------ This is where the code between GLFW and FreeGLUT are Different ---------------------------

/**
 * keyboard key was pressed on last frame <br>
 *  - Map Key is char for the keyboard key being used <br>
 *  - Value is bool was pressed on last frame
 * @note when key is uppercase it use for normally Special cases like using shift or up arrow
 */
std::map<char, bool> keyPressed;
/**
 * keyboard key is pressed this frame <br>
 *  - Map Key is char for the keyboard key being used <br>
 *  - Map Value is bool is pressed this frame
 * @note when key is uppercase it use for normally Special cases like using shift or up arrow
 */
std::map<char, bool> keyCurrentlyPressed;
/**
 * description on what the keyboard key used for <br>
 *  - Map Key is char for the keyboard key being used <br>
 *  - Map Value is std::string for the description
 * @note when key is uppercase it use for normally Special cases like using shift or up arrow
 */
std::map<char, std::string> keyDescription;

/**
 * On each frame it check for user input to toggle a flag
 */
void keyboard(bool setDiscrption) {
    if (setDiscrption) keyDescription['q'] = "Quit program";
    if (checkKey('q', GLFW_KEY_Q)) {
        tellWindowToClose();
    }

    bool sKey = checkKey('s', GLFW_KEY_S);
    bool shiftKeys = checkKey('S', GLFW_KEY_LEFT_SHIFT) || checkKey('S', GLFW_KEY_RIGHT_SHIFT);
    if (setDiscrption) keyDescription['s'] = "Show line view";
    if (sKey && !shiftKeys) {
        show_line = !show_line;
    }
    if (setDiscrption) keyDescription['S'] = "(SHIFT S) Show Line view but let the gpu hide hidden lines";
    if (sKey && shiftKeys) {
        show_line_new = !show_line_new;
    }

    if (setDiscrption) keyDescription['u'] = "Top view";
    if (setDiscrption) keyDescription['t'] = "Top view";
    if (checkKey('t', GLFW_KEY_T) || checkKey('u', GLFW_KEY_U)) {
        top_view_flag = !top_view_flag;
    }

    if (setDiscrption) keyDescription['r'] = "Rotate of camera";
    if (checkKey('r', GLFW_KEY_R)) {
        stop_rotate = !stop_rotate;
    }

    if (setDiscrption) keyDescription['F'] = "(F11) Full Screen";
    if (checkKey('F', GLFW_KEY_F11)) {
        setFullScreen(!isFullScreen);
    }
}

/**
 * check if key was press <br>
 * will only return true ones intel key not press on next check <br>
 * usefully for toggle things like show lines flag
 * @param key char often the key to store the state
 * @param GLFW_key the key that glfw recognise
 * @return false if still being press or not being press
 */
bool checkKey(char key, int GLFW_key) {
    bool returnValue;
    keyCurrentlyPressed[key] = glfwGetKey(window, GLFW_key ) == GLFW_PRESS;
    returnValue = (!keyPressed[key] && keyCurrentlyPressed[key]);
    keyPressed[key] = keyCurrentlyPressed[key];
    return returnValue;
}

/**
 * check window to be fullscreen
 * @param isFullScreenIn the state want
 */
void setFullScreen(bool isFullScreenIn) {
    if (isFullScreen == isFullScreenIn) {
        return;
    }
    if (!isFullScreenIn) {
        // auto unhide cursor when leaving full screen
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetWindowMonitor( window, nullptr,  0, 0, screenWidth, screenHeight, 0 );
        isFullScreen = false;
    } else {
        if (glfwGetWindowMonitor(window) == nullptr) {
            // auto hide cursor when full screen
            //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            GLFWmonitor* _monitor = glfwGetPrimaryMonitor();
            // get resolution of monitor
            const GLFWvidmode * mode = glfwGetVideoMode(_monitor);

            // switch to full screen
            glfwSetWindowMonitor( window, _monitor, 0, 0, mode->width, mode->height, 0 );
            isFullScreen = true;
        }
    }
}

/**
 * Auto update GL Viewport when window size changes <br>
 * This is a callback method for GLFW
 * @param thisWindow the window that updated
 * @param width the new width
 * @param height the new height
 */
void windowSizeChangeCallback([[maybe_unused]] GLFWwindow* thisWindow, int width, int height) {
    glScreenHeight = height;
    glScreenWidth = width;
    freeGLUTSizeUpdate = true;
    aspect = float(glScreenWidth) / float(glScreenHeight);
}

/**
 * Update Angle on each frame
 * @param deltaTime the time between frames
 */
void updateAngle(GLfloat deltaTime) {

    if (!stop_rotate) {
        rotateAngle += 2.75f * 10 * deltaTime;
    }

}

/**
 * Set the window flag to exit window
 */
void tellWindowToClose() {
    exitWindowFlag = true;
}

// ------------------ Main ---------------------------

/**
 * Start of the program MAIN <br>
 * Has the main loop
 * @return Exit code
 */
int main() {

    // Initialise GLFW
    fprintf(stdout, "Info: Initialise GLFW\n");
    if (!glfwInit()) {
        fprintf(stderr,"Error: initializing GLFW failed\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Info: Setting window hint's\n");
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#if OPENGL_DEBUG_FOR_GLFW
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    // Open a window and create its OpenGL context
    fprintf(stdout, "Info: Open a window and create its OpenGL context\n");
    char orginal_title[TITLE_LENGTH];
    strcpy(orginal_title, "GLFW - OpenGL - Basic");
    window = glfwCreateWindow(screenWidth, screenHeight, orginal_title, nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr,"Error: Failed to open GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // resize
    fprintf(stdout, "Info: Setup resize (size change callback)\n");
    glfwSetWindowSizeCallback(window, windowSizeChangeCallback);
    glfwGetWindowSize(window, &glScreenWidth, &glScreenHeight);
    // use so it knows the screen size the system wants
    // (for example when using high-res (4k) screen the system will likely want
    // double the size (Hi-DPI) to make it possible to see for the user)
    screenWidth = glScreenWidth;
    screenHeight = glScreenHeight;
    windowSizeChangeCallback(window, glScreenWidth, glScreenHeight);

    // fullscreen
    bool makeFullScreen = false;
    if (makeFullScreen) {
        setFullScreen(true);
    }

    // icon
    fprintf(stdout, "Info: Setup icon for the window\n");
    GLFWimage icons[1];
    icons[0].pixels = stbi_load(
            "res/icon/cube.png",
            &icons[0].width,
            &icons[0].height,
            nullptr, 4);
    if (icons[0].pixels == nullptr) {
        fprintf(stderr, "Error: Unable to load icon\n");
    } else {
        glfwSetWindowIcon(window, 1, icons);
        stbi_image_free(icons[0].pixels);
    }

    // Initialize devil
    fprintf(stdout,"Info: Initialize DevIL\n");
    ilInit();
    devILIsSetup = true;

    // Initialize GLEW
    fprintf(stdout,"Info: Initialize GLEW\n");
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Error: Failed to initialize GLEW\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

#if OPENGL_DEBUG_FOR_GLFW
    fprintf(stdout,"Info: Initialize GL Debug Output\n");
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    fprintf(stdout, "Info: Running Initialize method\n");
    Initialize();

    // GL info
    fprintf(stdout, "Info: GL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(stdout, "Info: GL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(stdout, "Info: GL Version (string) : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(stdout, "Info: GLSL Version : %s\n", glGetString(GL_VERSION));


    // Ensure we can capture the escape key being pressed below and any other keys
    fprintf(stdout, "Info: Setup user input mode\n");
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    // List Keys being used
    fprintf(stdout, "Info: Current keys being used\n");
    // called to set the keys in keyboard map
    keyboard(true);
    // Go throw the map and print each key being used
    for (std::pair<const char, std::string> node: keyDescription) {
        if (isupper(node.first)) { // Use uppercase for normally Special cases like using shift or up arrow
            fprintf(stdout, "Info:      - Special Key: %c : Description: %s\n", node.first, node.second.c_str());
        } else {
            fprintf(stdout, "Info:      -         Key: %c : Description: %s\n", node.first, node.second.c_str());
        }
    }

    fprintf(stdout, "Info: setting up variables for the loop\n");

    // DeltaTime variables
    GLfloat lastFrame = 0.0f;

    // FPS variables
    GLfloat lastTimeFPS = 0.0f;
    GLint numberOfFrames = 0;
    double fps;
    double avgFPS = 0.0;
    int qtyFPS = 0;

    fprintf(stdout,
            "Info: Start window loop with exit:%s and glfwWindowShouldClose(window):%s\n",
            exitWindowFlag ? "true" : "false",
            glfwWindowShouldClose(window) ? "true" : "false");
    while (!exitWindowFlag && !glfwWindowShouldClose(window)) {

        // Calculate delta time
        GLfloat currentFrame;
        currentFrame = static_cast<GLfloat>(glfwGetTime());
        GLfloat deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // FPS
        {
            GLfloat deltaTimeFPS = currentFrame - lastTimeFPS;
            numberOfFrames++;
            if (deltaTimeFPS >= 1.0f) {
                fps = static_cast<double>(numberOfFrames) / deltaTimeFPS;
                qtyFPS++;
                avgFPS += (fps - avgFPS) / qtyFPS;

                char title[TITLE_LENGTH];
                snprintf(title, TITLE_LENGTH - 1,
                         "%s - [FPS: %3.2f]", orginal_title,
                         fps);
                glfwSetWindowTitle(window, title);
                //fprintf(stdout, "Info: FPS: %f\n", fps);

                numberOfFrames = 0;
                lastTimeFPS = currentFrame;
            }
        }


        // Render
        Display();

        // Swap buffers
        glfwSwapBuffers(window);

        // Get evens (ex user input)
        glfwPollEvents();

        // check for user input to exit
        exitWindowFlag = glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS || exitWindowFlag;

        // check for user input
        keyboard(false);

        // update data (often angles of things)
        updateAngle(deltaTime);

    }
    fprintf(stdout, "Info: Avg FPS: %f\n", avgFPS);

    // Close OpenGL window and terminate GLFW
    fprintf(stdout, "Info: Close OpenGL window and terminate GLFW\n");
    glfwTerminate();

    return EXIT_SUCCESS;
}
#pragma clang diagnostic pop