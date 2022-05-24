#version 400

in vec4 Position;
in vec3 Normal;
in vec4 VertexColorOut;

uniform vec4 LightPosition;// Light position in eye coords.


out vec4 color;

void main() {


    vec3 s = normalize(vec3(LightPosition) - vec3(Position));
    float intensity = max(dot(s, Normal), 0.0);

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