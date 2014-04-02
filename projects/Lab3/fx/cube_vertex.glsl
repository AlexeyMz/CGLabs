#version 330

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 LightPosition_worldspace;
uniform vec4 LightColor;

layout(location = 0) in vec3 Position_modelspace;
layout(location = 2) in vec3 Normal_modelspace;
layout(location = 3) in vec2 TexCoord0;
layout(location = 4) in vec3 Tangent_modelspace;
layout(location = 5) in vec3 Bitangent_modelspace;

out vec2 FragTexCoord;
out vec3 LightDirection_cameraspace;
out vec3 vNormal_modelspace;
out vec3 vTangent_modelspace;
out vec3 vBitangent_modelspace;

void main()
{
	vec3 position_worldspace = (Model * vec4(Position_modelspace, 1)).xyz;
    vec3 lightDir_worldspace = normalize(LightPosition_worldspace - position_worldspace);
    
    gl_Position = Projection * View * vec4(position_worldspace, 1);
	FragTexCoord = TexCoord0;

	// Normal of the the vertex, in camera space
	// Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
	LightDirection_cameraspace = (View * vec4(lightDir_worldspace, 0)).xyz;

	vNormal_modelspace = Normal_modelspace;
	vTangent_modelspace = Tangent_modelspace;
	vBitangent_modelspace = Bitangent_modelspace;
}