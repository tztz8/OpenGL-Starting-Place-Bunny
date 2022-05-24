#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
/**
 * Main
 */

//          --- Libraries ---

// Normal Lib
#include <cstdio>
#include <cstdlib>
#include <string>

// Include GLEW
#include <GL/glew.h>
#pragma comment(lib, "opengl32.lib")
#include <GL/freeglut.h>

// image lib (cscd377)
#include <IL/il.h>
//#include <IL/ilu.h>
//#include <IL/ilut.h>

// Modified GLM (obj file loaders)
#include "GLM.h"

// Math Lib
#define GLM_FORCE_RADIANS
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective

// h file of this assignment
#include "Cube.h"

//          --- Filled's ---

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

// Angle (for this It's for the light)
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
    }

    glUseProgram(p);

    return p;

}

/**
 * Load Texture
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @note (used devil to load the image)
 * @note do not forget to uncommitted the include lib at the top and uncommitted the setup in main
 * @param filename path to image file
 * @return GL Texture ID
 */
unsigned int loadTexture(const char* filename) {
    if (devILIsSetup) {
        ILboolean success;
        unsigned int imageID;
        ilGenImages(1, &imageID);

        ilBindImage(imageID); /* Binding of DevIL image name */
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        success = ilLoadImage((ILstring)filename);

        if (!success) {
            fprintf(stderr, "Error: loadTexture: Couldn't load the following texture file: %s\n", filename);
            // The operation was not sucessfull hence free image and texture
            ilDeleteImages(1, &imageID);
            return 0;
        }

        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

        GLuint tid;
        glGenTextures(1, &tid);
        glBindTexture(GL_TEXTURE_2D, tid);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        /* Because we have already copied image data into texture data
        we can release memory used by image. */

        ilDeleteImages(1, &imageID);
        fprintf(stdout, "Info: loadTexture: \"%s\" is ready\n", filename);
        return tid;
    } else {
        fprintf(stderr, "Error: loadTexture: DevIL is not setup\n");
        return 0;
    }
}

/**
 * Update All The Vertex Normals
 * @param vertices the vertices used to cal the norms
 * @param norms the norms that will be updated (WARNING the data will be overridden)
 * @param indices to know which vertices used to cal the norms
 * @param numNormals the number of normals
 * @param numIndices the number of indices
 * @warning the data in norms will be overridden
 */
void updateVertexNormals(vec3* vertices, vec3* norms, const GLuint* indices,
                         GLuint numNormals, GLuint numIndices) {

    glm::vec3 p1, p2, p3, n;

    for (int i = 0; i < numNormals; i++) {
        norms[i] = glm::vec3(0.0, 0.0, 0.0);
    }

    for (int index = 0; index < numIndices; index+=3) {

        p1 = vertices[indices[index + 0]];
        p2 = vertices[indices[index + 1]];
        p3 = vertices[indices[index + 2]];

        n = normalize(cross((p2 - p1), (p3 - p1)));

        norms[indices[index + 0]] += n;
        norms[indices[index + 1]] += n;
        norms[indices[index + 2]] += n;
    }

    for (int i = 0; i < numNormals; i++) {
        norms[i] = glm::normalize(norms[i]);
    }
}

/**
 * Center and make the model fit in a -1 to 1 box
 * @param vertices the vertices to update
 * @param numVertices the number of vertices
 * @warning the data in vertices will be overridden
 */
void unitizeModel(glm::vec3 vertices[], GLuint numVertices) {
    // Step 1: Compute the maximum and minimum of x, y, and z
    // coordinates of the model’s vertices.
    float min_x, max_x, min_y, max_y, min_z, max_z;
    min_x = max_x = vertices[0].x;
    min_y = max_y = vertices[0].y;
    min_z = max_z = vertices[0].z;

    // finding the min and max for xyz
    for (int i = 0; i < numVertices; ++i) {
        glm::vec3 vertex = vertices[i];
        if (vertex.x < min_x) {
            min_x = vertex.x;
        } else if (vertex.x > max_x) {
            max_x = vertex.x;
        }

        if (vertex.y < min_y) {
            min_y = vertex.y;
        } else if (vertex.y > max_y) {
            max_y = vertex.y;
        }

        if (vertex.z < min_z) {
            min_z = vertex.z;
        } else if (vertex.z > max_z) {
            max_z = vertex.z;
        }
    }

    // Step 2: Calculate the center as follows:
    float center_x = (max_x + min_x) / 2;
    float center_y = (max_y + min_y) / 2;
    float center_z = (max_z + min_z) / 2;

    // Step 3: Calculate the width, height, and depth of the model.
    float width = std::abs(max_x - min_x);
    float height = std::abs(max_y - min_y);
    float depth = std::abs(max_z - min_z);

    // Step 4: Calculate the scale factor!.
    float scale = glm::max(depth, glm::max(width, height));

    for (int i = 0; i < numVertices; i++) {
        // Step 5: Center the model at the origin!
        // moving points to center of the screen
        vertices[i].x -= center_x;
        vertices[i].y -= center_y;
        vertices[i].z -= center_z;

        // Step 6:Divide each coordinate by the scale factor determined
        // in Step 4! This will fit the model in a bounding box of width,
        // height, and depth 1 each extending from -0.5 to +0.5
        // scale the model to fit in a box whose width, height, and depth extend from -0.5 to 0.5.
        vertices[i].x /= scale;
        vertices[i].y /= scale;
        vertices[i].z /= scale;

        // Step 7: Multiply each coordinate by 2.0! This will fit the
        // model in a bounding box of width, height, and depth 2 each
        // extending from -1.0 to +1.0

        //scale the model to fit in a box whose width, height, and depth extend from -1.0 to 1.0.
        vertices[i].x *= 2;
        vertices[i].y *= 2;
        vertices[i].z *= 2;
    }
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
    program = initShaders("shader.vert", "shader.frag");

    // Use the shader program
    glUseProgram(program);

    // attribute indices
    model_matrix = glm::mat4(1.0f);
    view_matrix_loc = glGetUniformLocation(program, "view_matrix");
    matrix_loc = glGetUniformLocation(program, "model_matrix");
    projection_matrix_loc = glGetUniformLocation(program, "projection_matrix");
    light_position_loc = glGetUniformLocation(program, "LightPosition");
    glUniform1i(glGetUniformLocation(program, "Tex1"), 0);

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
void Display()
{
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

    // Size of Points if drawn
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
    glutSwapBuffers(); // This line is different in glfw and in main instead
}

// ------------------ This is where the code between GLFW and FreeGLUT are Vary Different ---------------------------

/**
 * called when key on the keyboard is pressed
 * @param key the key being pressed
 * @param x x of "mouse location in window relative coordinates when the key was pressed"
 * @param y y of "mouse location in window relative coordinates when the key was pressed"
 */
void keyboard(unsigned char key, [[maybe_unused]] int x, [[maybe_unused]] int y){

    switch (key){
        case 'q':case 'Q':
            exit(EXIT_SUCCESS);
            break;
        case 's':
            show_line = !show_line;
            break;
        case 'S':
            show_line_new = !show_line_new;
            break;
        case 't':
        case 'T':
        case 'u':
        case 'U':
            top_view_flag = !top_view_flag;
            break;
        case 'r':
        case 'R':
            stop_rotate = !stop_rotate;
            break;
        default: ;
            // Do nothing
    }
    glutPostRedisplay();
}

/**
 * Is called when the window is resize
 * @param width the new width of the window
 * @param height the new height of the window
 */
void Reshape(int width, int height) {
    glViewport(0, 0, width, height);
    aspect = float(width) / float(height);
    glutPostRedisplay();
}

/**
 * Default method to use with glutTimerFunc to update things often rotate
 * @param n witch thing we are updating
 */
void rotate(int n) {
    switch (n) {
        case 1:
            if (!stop_rotate) {
                rotateAngle += 2.75f;
            }

            glutPostRedisplay(); // Redraw the screen
            // restart timer
            glutTimerFunc(100, rotate, 1); // update forever (not just ones)
            break;
        default: ;
            // do nothing
    }

}

// ------------------ Main ---------------------------

/**
 * Start of the program
 * @param argc number of arguments
 * @param argv pointer to array of arguments (string (array of char))
 * @return Exit code (0 is often means no problems)
 */
int main(int argc, char** argv){
    fprintf(stdout, "Info: Initialise GLUT\n");
    glutInit(&argc, argv);
    fprintf(stdout, "Info: Setting GLUT Display modes\n");
    glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL);
    fprintf(stdout, "Info: Initialise a window size\n");
    glutInitWindowSize(512, 512);

    fprintf(stdout, "Info: Create a window\n");
    glutCreateWindow("FreeGLUT - OpenGL - Basic");

    fprintf(stdout,"Info: Initialize GLEW\n");
    if (glewInit()) {
        fprintf(stderr, "Error: Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    fprintf(stdout,"Info: Initialize DevIL\n");
    ilInit();
    devILIsSetup = true;

    fprintf(stdout, "Info: Running Initialize method\n");
    Initialize(); // Our Initialize method

    // GL info
    fprintf(stdout, "Info: GL Vendor : %s\n", glGetString(GL_VENDOR));
    fprintf(stdout, "Info: GL Renderer : %s\n", glGetString(GL_RENDERER));
    fprintf(stdout, "Info: GL Version (string) : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(stdout, "Info: GLSL Version : %s\n", glGetString(GL_VERSION));

    fprintf(stdout, "Info: GLUT Set display func\n");
    glutDisplayFunc(Display); // Tell glut our display method
    fprintf(stdout, "Info: GLUT Set keyboard func\n");
    glutKeyboardFunc(keyboard); // Tell glut our keyboard method
    fprintf(stdout, "Info: GLUT Set reshape func\n");
    glutReshapeFunc(Reshape); // Tell glut our reshape method
    fprintf(stdout, "Info: GLUT Set rotate func (rotateAngle)\n");
    glutTimerFunc(100, rotate, 1); // First timer for rotate camera
    fprintf(stdout, "Info: GLUT Load Main Loop\n");
    glutMainLoop(); // Start glut infinite loop

    fprintf(stdout, "Info: Exiting\n");
    return EXIT_SUCCESS;
}
#pragma clang diagnostic pop