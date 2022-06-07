#version 400

// from model
layout (location = 0) in vec4 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

// to the frag shader
out vec4 Position; //eyeSpacePosition
out vec3 Normal; //eyeSpaceNormal
out vec2 TexCoord;

// from our program
uniform mat4 view_matrix;
uniform mat4 projection_matrix;
uniform mat4 model_matrix;

void main()
{
   Normal = normalize(mat3(view_matrix*model_matrix)* VertexNormal);

   Position = view_matrix*model_matrix *VertexPosition;

   TexCoord = VertexTexCoord;

   gl_Position = projection_matrix* view_matrix*model_matrix *VertexPosition;
}