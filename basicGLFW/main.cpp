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

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// image lib (cscd377)
//#include <IL/il.h>
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

// h file of this assignment
#include "Cube.h"

//          --- Filled's ---

// glfw window id
GLFWwindow* window;

// Bool to know when to exit
bool exitWindowFlag = false;

// title info
#define TITLE_LENGTH 100

// devIL is setup flag
bool devILIsSetup = false; // DO NOT CHANGE

// Window GL variables
/**
 * Aspect ratio <br>
 * Proportion between the width and the height of the window
 */
GLfloat aspect = 0.0;

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

//          --- Pre-Def Methods ---

/**
 * Set the window flag to exit window
 */
void tellWindowToClose();

//          --- Methods ---

/**
 * Read A file and out put a file a char list
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param filename path to the file
 * @return pointer to a char list (String)
 */
char* ReadFile(const char* filename) {

    FILE* infile;
#ifdef WIN32
    fopen_s(&infile, filename, "rb");
#else
    infile = fopen(filename, "rb");
#endif


    if (!infile) {
        fprintf(stderr, "Error: ReadFile: Unable to open file %s\n", filename);
        return nullptr;
    }

    fseek(infile, 0, SEEK_END);
    int len = static_cast<int>(ftell(infile));
    fseek(infile, 0, SEEK_SET);
    char* source = (char*)malloc(len + 1);
    if (source == nullptr) {
        fprintf(stderr, "Error: ReadFile: Unable to get memory to read file %s\n", filename);
        tellWindowToClose();
        return nullptr;
    }
    fread(source, 1, len, infile);
    fclose(infile);
    source[len] = 0;
    fprintf(stdout, "Info: ReadFile: \"%s\" is ready\n", filename);
    return (source);

}

/**
 * Initialize Shaders
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param v_shader the vertex shader path
 * @param f_shader the fragment shader path
 * @return a gl program object
 */
GLuint initShaders(const char* v_shader, const char* f_shader) {

    GLuint p = glCreateProgram();

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    const char * vs = ReadFile(v_shader);
    const char * fs = ReadFile(f_shader);

    glShaderSource(v, 1, &vs, nullptr);
    glShaderSource(f, 1, &fs, nullptr);

    free((char*)vs);
    free((char*)fs);

    glCompileShader(v);

    GLint compiled;

    glGetShaderiv(v, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLsizei len;
        glGetShaderiv(v, GL_INFO_LOG_LENGTH, &len);

        char* log = (char*)malloc(len + 1);

        if (log == nullptr) {
            fprintf(stderr, "Error: initShaders: Was not able to get memory to get error code for compiled shader\n");
            exit(EXIT_FAILURE);
        }

        glGetShaderInfoLog(v, len, &len, log);

        fprintf(stderr, "Error: initShaders: Vertex Shader compilation failed: %s\n", log);

        free(log);

        tellWindowToClose();
    }

    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {

        GLsizei len;
        glGetShaderiv(f, GL_INFO_LOG_LENGTH, &len);
        char* log = (char*)malloc(len + 1);

        if (log == nullptr) {
            fprintf(stderr, "Error: initShaders: Was not able to get memory to get error code for compiled shader\n");
            exit(EXIT_FAILURE);
        }

        glGetShaderInfoLog(f, len, &len, log);
        fprintf(stderr, "Error: initShaders: Vertex Shader compilation failed: %s\n", log);
        free(log);

        tellWindowToClose();
    }

    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint linked;

    glGetProgramiv(p, GL_LINK_STATUS, &linked);

    if (!linked) {

        GLsizei len;
        glGetProgramiv(p, GL_INFO_LOG_LENGTH, &len);
        char* log = (char*)malloc(len + 1);

        if (log == nullptr) {
            fprintf(stderr, "Error: initShaders: Was not able to get memory to get error code for compiled shader\n");
            exit(EXIT_FAILURE);
        }

        glGetProgramInfoLog(p, len, &len, log);
        fprintf(stderr, "Error: initShaders: Shader linking failed: %s\n", log);
        free(log);

        tellWindowToClose();
    }

    glUseProgram(p);

    return p;

}

///**
// * Load Texture
// * @note code from Yasmin and commit and some modification make by Timbre Freeman
// * @note (used devil to load the image)
// * @note do not forget to uncommitted the include lib at the top and uncommitted the setup in main
// * @param filename path to image file
// * @return GL Texture ID
// */
//unsigned int loadTexture(const char* filename) {
//    if (devILIsSetup) {
//        ILboolean success;
//        unsigned int imageID;
//        ilGenImages(1, &imageID);
//
//        ilBindImage(imageID); /* Binding of DevIL image name */
//        ilEnable(IL_ORIGIN_SET);
//        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
//        success = ilLoadImage((ILstring)filename);
//
//        if (!success) {
//            fprintf(stderr, "Error: loadTexture: Couldn't load the following texture file: %s\n", filename);
//            // The operation was not sucessfull hence free image and texture
//            ilDeleteImages(1, &imageID);
//            tellWindowToClose();
//            return 0;
//        }
//
//        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
//
//        GLuint tid;
//        glGenTextures(1, &tid);
//        glBindTexture(GL_TEXTURE_2D, tid);
//        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
//                     GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//
//        glBindTexture(GL_TEXTURE_2D, 0);
//
//        /* Because we have already copied image data into texture data
//        we can release memory used by image. */
//
//        ilDeleteImages(1, &imageID);
//        fprintf(stdout, "Info: loadTexture: \"%s\" is ready\n", filename);
//        return tid;
//    } else {
//        fprintf(stderr, "Error: loadTexture: DevIL is not setup\n");
//        tellWindowToClose();
//        return 0;
//    }
//}

/**
 * Called set setup open gl things (for example making the models)
 */
void Initialize(){
    // Create the program for rendering the model
    program = initShaders("shader.vert", "shader.frag");

    // Check if making the shader work or not // This is not in FreeGLUT as does need an exit flag
    if (exitWindowFlag) {
        return;
    }

    // Use the shader program
    glUseProgram(program);

    // attribute indices
    model_matrix = glm::mat4(1.0f);
    view_matrix_loc = glGetUniformLocation(program, "view_matrix");
    matrix_loc = glGetUniformLocation(program, "model_matrix");
    projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
    light_position_loc = glGetUniformLocation(program, "LightPosition");

    // Set Clear Color (background color)
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // setup Cube
    createCube();
}

/**
 * Called for every frame to draw on the screen
 */
void Display() {
    // Clear
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
    glUniform4fv(light_position_loc, 1, (GLfloat*)&light_position_camera[0]);

    // update projection matrix (useful when the window resize)
    projection_matrix = glm::perspective(glm::radians(45.0f), aspect, 0.3f, 100.0f);
    glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, (GLfloat*)&projection_matrix[0]);

    // Draw things

    // Draw Cube
    model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
    glUniformMatrix4fv(matrix_loc, 1, GL_FALSE, (GLfloat*)&model_matrix[0]);
    drawCube();

    // End of Draw things
    glFlush();
}

// ------------------ This is where the code between GLFW and FreeGLUT are Different ---------------------------

/**
 * key was pressed on last frame <br>
 *  - Key is char <br>
 *  - Value is bool
 * @note when key is uppercase it use for normally Special cases like using shift or up arrow
 */
std::map<char, bool> keyPressed;
/**
 * key is pressed this frame <br>
 *  - Key is char <br>
 *  - Value is bool
 * @note when key is uppercase it use for normally Special cases like using shift or up arrow
 */
std::map<char, bool> keyCurrentlyPressed;

/**
 * On each frame it check for user input to toggle a flag
 */
void keyboard() {
    keyCurrentlyPressed['q'] = glfwGetKey(window, GLFW_KEY_Q ) == GLFW_PRESS;
    if (!keyPressed['q'] && keyCurrentlyPressed['q']) {
        tellWindowToClose();
    }
    keyPressed['q'] = keyCurrentlyPressed['q'];

    keyCurrentlyPressed['s'] = glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS;
    if (!keyPressed['s'] && keyCurrentlyPressed['s']) {
        show_line = !show_line;
    }
    keyPressed['s'] = keyCurrentlyPressed['s'];

    keyCurrentlyPressed['S'] =
            keyCurrentlyPressed['s'] && // glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS
            (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
             glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
    if (!keyPressed['S'] && keyCurrentlyPressed['S']) {
        show_line_new = !show_line_new;
        // NOTE that s was press, so it also toggles the show_line flag above
    }
    keyPressed['S'] = keyCurrentlyPressed['S'];

    keyCurrentlyPressed['u'] = glfwGetKey(window, GLFW_KEY_U ) == GLFW_PRESS;
    keyCurrentlyPressed['t'] = glfwGetKey(window, GLFW_KEY_T ) == GLFW_PRESS;
    if ((!keyPressed['t'] && keyCurrentlyPressed['t']) ||
        (!keyPressed['u'] && keyCurrentlyPressed['u'])) {
        top_view_flag = !top_view_flag;
    }
    keyPressed['t'] = keyCurrentlyPressed['t'];
    keyPressed['u'] = keyCurrentlyPressed['u'];

    keyCurrentlyPressed['r'] = glfwGetKey(window, GLFW_KEY_R ) == GLFW_PRESS;
    if (!keyPressed['r'] && keyCurrentlyPressed['r']) {
        stop_rotate = !stop_rotate;
    }
    keyPressed['r'] = keyCurrentlyPressed['r'];
}

/**
 * Auto update GL Viewport when window size changes <br>
 * This is a callback method for GLFW
 * @param thisWindow the window that updated
 * @param width the new width
 * @param height the new height
 */
void windowSizeChangeCallback([[maybe_unused]] GLFWwindow* thisWindow, int width, int height) {
    glViewport(0, 0, width, height);
    aspect = float(width) / float(height);
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

    // Open a window and create its OpenGL context
    fprintf(stdout, "Info: Open a window and create its OpenGL context\n");
    char orginal_title[TITLE_LENGTH];
    strcpy(orginal_title, "GLFW - OpenGL - Basic");
    window = glfwCreateWindow(512, 512, orginal_title, nullptr, nullptr);
    if (window == nullptr) {
        fprintf(stderr,"Error: Failed to open GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);

    // resize
    fprintf(stdout, "Info: Setup resize (size change callback)\n");
    glfwSetWindowSizeCallback(window, windowSizeChangeCallback);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    windowSizeChangeCallback(window, width, height);

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
//    fprintf(stdout,"Info: Initialize DevIL\n");
//    ilInit();
//    devILIsSetup = true;

    // Initialize GLEW
    fprintf(stdout,"Info: Initialize GLEW\n");
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Error: Failed to initialize GLEW\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

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
    keyboard();
    // Go throw the map and print each key being used
    for (std::pair<const char, bool> node: keyPressed) {
        if (isupper(node.first)) { // Use uppercase for normally Special cases like using shift or up arrow
            fprintf(stdout, "Info:      - Special Key: %c\n", node.first);
        } else {
            fprintf(stdout, "Info:      -         Key: %c\n", node.first);
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
                fps = double(numberOfFrames) / deltaTimeFPS;
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
        keyboard();

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