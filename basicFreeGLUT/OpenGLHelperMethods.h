#ifndef OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H
#define OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H

// Include GLEW
#include <GL/glew.h>
// freeglut to get APIENTRY
#include <GL/freeglut.h>
#ifndef APIENTRY
#define APIENTRY GLAPIENTRY
#endif

// Math Lib
#define GLM_FORCE_RADIANS
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/geometric.hpp>

#define OPENGL_DEBUG_FOR_GLUT true

/**
 * Read A file and out put a file a char list
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param filename path to the file
 * @return pointer to a char list (String)
 */
char* ReadFile(const char* filename);

/**
 * Initialize Shaders
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param s paths (as char strings) of the shader files, in this order <br>
 *          - Vertex <br>
 *          - Fragment <br>
 *          - Geometry <br>
 * @param count number of shader files
 * @warning if count is above 3 it will exit
 * @return shader program
 */
GLuint initShaders(const char* s[], int count);

/**
 * Initialize Shaders
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param v_shader the vertex shader path
 * @param f_shader the fragment shader path
 * @return a gl program object
 */
GLuint initShaders(const char* v_shader, const char* f_shader);

/**
 * Generates a texture that is suited for attachments to a framebuffer
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 */
GLuint generateAttachmentTexture();

/**
 * Load Texture
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @note (used devil to load the image)
 * @note do not forget to uncommitted the include lib at the top and uncommitted the setup in main
 * @param filename path to image file
 * @return GL Texture ID
 */
unsigned int loadTexture(const char* filename);

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
        const char* posZ, const char* negZ);

/**
 * Get the tangents from the vertices, normals, texture and indices
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param vertices the vertices (points)
 * @param normals the normals
 * @param tangents the tangents (warning tangents will be overwritten)
 * @param numvertices the number of vertices
 * @param numindices the number of indices
 * @param indices the indices
 * @param tex the texture coordinates
 * @warning tangents will be overwritten
 */
void updateVertexTangents(const glm::vec4* vertices, const glm::vec3* normals, glm::vec4* tangents, int numvertices, int numindices,  const GLushort* indices, const glm::vec2* tex);

/**
 * Get the tangents from the vertices, normals, texture and indices
 * @note code from Yasmin and commit and some modification make by Timbre Freeman
 * @param vertices the vertices (points)
 * @param normals the normals
 * @param tangents the tangents (warning tangents will be overwritten)
 * @param numvertices the number of vertices
 * @param numindices the number of indices
 * @param indices the indices
 * @param tex the texture coordinates
 * @warning tangents will be overwritten
 */
void updateVertexTangents(const glm::vec4* vertices, const glm::vec3* normals, glm::vec4* tangents, int numvertices, int numindices,  const GLuint* indices, const glm::vec2* tex);

/**
 * Update All The Vertex Normals
 * @param vertices the vertices used to cal the norms (points)
 * @param norms the norms that will be updated (WARNING the data will be overridden)
 * @param indices to know which vertices used to cal the norms
 * @param numNormals the number of normals
 * @param numIndices the number of indices
 * @warning the data in norms will be overridden
 */
void updateVertexNormals(const glm::vec3* vertices, glm::vec3* norms, const GLuint* indices,
                         GLuint numNormals, GLuint numIndices);

/**
 * Update All The Vertex Normals
 * @param vertices the vertices used to cal the norms (points)
 * @param norms the norms that will be updated (WARNING the data will be overridden)
 * @param indices to know which vertices used to cal the norms
 * @param numNormals the number of normals
 * @param numIndices the number of indices
 * @warning the data in norms will be overridden
 */
void updateVertexNormals(const glm::vec3* vertices, glm::vec3* norms, const GLushort* indices,
                         GLuint numNormals, GLuint numIndices);

/**
 * Center and make the model fit in a -1 to 1 box
 * @param vertices the vertices to update (points)
 * @param numVertices the number of vertices
 * @warning the data in vertices will be overridden
 */
void unitizeModel(glm::vec3 vertices[], GLuint numVertices);

#if OPENGL_DEBUG_FOR_GLUT
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
                            [[maybe_unused]] const void *userParam);
#endif

#endif //OPENGL_STARTING_PLACE_BUNNY_OPENGLHELPERMETHODS_H
