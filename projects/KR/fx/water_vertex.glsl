#version 330

layout(location = 0) in vec3 Vertex;
out vec2 coord;

void main() {
    coord = Vertex.xy * 0.5 + 0.5;
    gl_Position = vec4(Vertex.xyz, 1.0);
}