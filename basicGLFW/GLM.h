#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <GL/freeglut.h>

#ifndef M_PI
#define M_PI 3.14159265f
#endif


#define GLM_FORCE_RADIANS
#define TWOPI 2*3.141592653589793238//4626433832795

#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale, glm::perspective


//using namespace glm;


/* GLMtriangle: Structure that defines a triangle in a model.
 */


/* GLMmodel: Structure that defines a model.
 */
using GLMmodel = struct _GLMmodel {

    GLuint numvertices;         /* number of vertices in model */
    glm::vec3 *vertices;         /* array of vertices  */


    GLuint numnormals;          /* number of normals in model */
    glm::vec3 *normals;        /* array of normals */

    GLuint numtextures;          /* number of textures in model */
    glm::vec2 *textures;        /* array of tex coordinates */

    GLuint numtriangles;    /* number of triangles in model */


    GLuint numindices;
    GLuint *indices;


};





GLvoid
glmDelete(GLMmodel* model);

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
 * Returns a pointer to the created object which should be free'd with
 * glmDelete().
 *
 * filename - name of the file containing the Wavefront .OBJ format data.  
 */
GLMmodel*
glmReadOBJ(const char* filename);



