#version 400

layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec4 VertexColorIn;
layout (location = 2) in vec3 VertexNormal;


out vec4 Position;
out vec3 Normal;
out vec4 VertexColorOut;


uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 model_matrix;

void main()
{
   Normal = normalize(mat3(view_matrix*model_matrix)* VertexNormal);

   Position = view_matrix*model_matrix *VertexPosition;

   VertexColorOut = VertexColorIn;

   gl_Position = projection_matrix* view_matrix*model_matrix *VertexPosition;
}