#version 400

// from our vert shader
in vec4 Position;
in vec3 Normal;
in vec2 TexCoord;

// from our program
uniform vec4 LightPosition;// Light position in eye coords.

uniform sampler2D Tex1;

// to our gpu
out vec4 color;

void main() {


    vec3 s = normalize(vec3(LightPosition) - vec3(Position));
    float intensity = max(dot(s, Normal), 0.0);

    vec4 VertexColorOut = texture( Tex1, TexCoord );

    if (intensity > 0.95) {
        color = vec4(vec3(VertexColorOut * 0.95), 1.0);
    } else if (intensity > 0.75) {
        color = vec4(vec3(VertexColorOut * 0.75), 1.0);
    } else if (intensity > 0.5) {
        color = vec4(vec3(VertexColorOut * 0.50), 1.0);
    } else if (intensity > 0.25) {
        color = vec4(vec3(VertexColorOut * 0.25), 1.0);
    } else {
        color = vec4(vec3(VertexColorOut * 0.10), 1.0);
    }
    //color = VertexColorOut;
}
