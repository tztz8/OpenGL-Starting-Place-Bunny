//
// Created by tztz8 on 6/2/22.
//

#ifndef OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H
#define OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H

#include <cstdio>
#include <cstdlib>
#include <malloc.h>

// Include GLEW
#include <GL/glew.h>

// image lib
#include <IL/il.h>

// Math Lib
#define GLM_FORCE_RADIANS
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>

#include "main.h"

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
            tellWindowToClose();
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
        tellWindowToClose();
        return 0;
    }
}

/**
 * Load Cube Map Texture
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @note (used devil to load the image)
 * @note do not forget to uncommitted the include lib at the top and uncommitted the setup in main
 * @param posX path to posX image file
 * @param negX path to negX image file
 * @param posY path to posY image file
 * @param negY path to negY image file
 * @param posZ path to posZ image file
 * @param negZ path to negZ image file
 * @return GL Texture ID
 */
unsigned int loadCubeMapTexture(
        const char* posX, const char* negX,
        const char* posY, const char* negY,
        const char* posZ, const char* negZ) {
    if (devILIsSetup) {
        ILboolean success;
        unsigned int imageID;
        GLuint textureID;
        GLuint faceTarget[] = {
                GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        };
        const char* files[6];

        files[0] = posX;
        files[1] = negX;
        files[2] = posY;
        files[3] = negY;
        files[4] = posZ;
        files[5] = negZ;

        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &textureID); /* Texture name generation */
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        // Load Textures for Cube Map

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        ilGenImages(1, &imageID);
        ilBindImage(imageID); /* Binding of DevIL image name */

        for (int i = 0; i < 6; ++i) {
            ilEnable(IL_ORIGIN_SET);
            ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
            success = ilLoadImage((ILstring)files[i]);

            if (!success) {
                fprintf(stderr, "Error: loadCubeMapTexture: Couldn't load texture file \"%s\"\n", files[i]);
                // The operation was not sucessfull hence free image and texture
                ilDeleteImages(1, &imageID);
            }

            /* Convert image to RGBA */
            ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

            /* Create and load textures to OpenGL */
            glTexImage2D(faceTarget[i], 0, GL_RGBA,
                         ilGetInteger(IL_IMAGE_WIDTH),
                         ilGetInteger(IL_IMAGE_HEIGHT),
                         0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

            fprintf(stdout,"Info: loadCubeMapTexture: Texture Loaded: %s\n", files[i]);
        }
        fprintf(stdout,"Info: loadCubeMapTexture: Cube Map Loaded Successfully.\n");


        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        /* Because we have already copied image data into texture data we can release memory used by image. */
        ilDeleteImages(1, &imageID);

        // add information to the log

        return textureID;
    } else {
        fprintf(stderr, "Error: loadCubeMapTexture: DevIL is not setup\n");
        tellWindowToClose();
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
void updateVertexNormals(glm::vec3* vertices, glm::vec3* norms, const GLuint* indices,
                         GLuint numNormals, GLuint numIndices) {

    glm::vec3 p1, p2, p3, n;

    for (int i = 0; i < numNormals; i++) {
        norms[i] = glm::vec3(0.0, 0.0, 0.0);
    }

    for (int index = 0; index < numIndices; index+=3) {

        p1 = vertices[indices[index + 0]];
        p2 = vertices[indices[index + 1]];
        p3 = vertices[indices[index + 2]];

        n = glm::normalize(cross((p2 - p1), (p3 - p1)));

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

#if OPENGL_DEBUG_FOR_GLFW
/**
 * OpenGL Debug message callback to output
 * @param source Where the error came from
 * @param type The type of error
 * @param id the id of the error
 * @param severity how bad the error was
 * @param length Unknown (my guest is the size of message)
 * @param message the message of the error
 * @param userParam Unknown
 * @see https://learnopengl.com/In-Practice/Debugging
 * @warning if severity is high tell glfw to close
 */
void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            [[maybe_unused]] GLsizei length,
                            const char *message,
                            [[maybe_unused]] const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cout << "Info: OpenGL Debug incoming (ID:" << id << ")" << std::endl;

    std::string sourceMessage;
    std::string typeMessage;
    std::string severityMessage;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             sourceMessage = "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceMessage = "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceMessage = "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceMessage = "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     sourceMessage = "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           sourceMessage = "Source: Other"; break;
        default:                              sourceMessage = "Source: Unknown"; break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               typeMessage = "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeMessage = "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeMessage = "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         typeMessage = "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         typeMessage = "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              typeMessage = "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          typeMessage = "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           typeMessage = "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               typeMessage = "Type: Other"; break;
        default:                                typeMessage = "Type: Unknown"; break;
    }

    FILE* output;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severityMessage = "Severity: high"; output = stderr; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severityMessage = "Severity: medium"; output = stderr; break;
        case GL_DEBUG_SEVERITY_LOW:          severityMessage = "Severity: low"; output = stdout; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severityMessage = "Severity: notification"; output = stdout; break;
        default:                             severityMessage = "Severity: Unknown"; output = stderr; break;
    }

    fprintf(output, "-------OpenGL Debug--------\n"
                    "Debug message (ID:%u): %s\n"
                    "%s\n%s\n%s\n\n",
            id, message,
            sourceMessage.c_str(),
            typeMessage.c_str(),
            severityMessage.c_str());
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        tellWindowToClose();
    }
}
#endif

#endif //OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H
