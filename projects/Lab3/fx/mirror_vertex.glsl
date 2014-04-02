#version 330

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;

layout(location = 0) in vec3 Position_modelspace;
layout(location = 2) in vec3 Normal_modelspace;

out vec3 Position_worldspace;
out vec3 Position_cameraspace;
out vec3 Normal_cameraspace;

void main()
{
	Position_worldspace = (Model * vec4(Position_modelspace, 1)).xyz;
	Position_cameraspace = (View * vec4(Position_worldspace, 1)).xyz;
	Normal_cameraspace = (View * Model * vec4(Normal_modelspace, 1)).xyz;
	gl_Position = Projection * vec4(Position_cameraspace, 1);
}