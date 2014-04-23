#version 330

uniform mat4 MVP;
uniform sampler2D water;

layout(location = 0) in vec3 Vertex;
out vec3 position;

void main() {
	vec4 info = texture2D(water, Vertex.xy * 0.5 + 0.5);
	position = Vertex.xzy;
	position.y += info.r;
	gl_Position = MVP * vec4(position, 1.0);
}