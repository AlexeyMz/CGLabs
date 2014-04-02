#version 330

uniform mat4 View;
uniform sampler2D FloorTexture;
uniform float FloorTextureScaling;
uniform float FloorHeight;

smooth in vec3 Position_worldspace;
smooth in vec3 Position_cameraspace;
smooth in vec3 Normal_cameraspace;

layout(location = 0) out vec4 fragmentColor;

void main()
{
	vec3 incidentEye = normalize(Position_cameraspace);
	vec3 normal = normalize(Normal_cameraspace);
	vec3 reflected_cameraspace = reflect(incidentEye, normal);
	vec3 reflected_worldspace = normalize((inverse(View) * vec4(reflected_cameraspace, 0)).xyz);

	float distance = Position_worldspace.y - FloorHeight;
	if (sign(distance) == sign(reflected_worldspace.y))
		fragmentColor = vec4(0, 0, 0, 1);
	else {
		vec2 texPos_worldspace = (distance / reflected_worldspace.y) * reflected_worldspace.xz + Position_worldspace.xz;
		vec2 texPos_texspace = FloorTextureScaling * texPos_worldspace;
		fragmentColor = texture2D(FloorTexture, texPos_texspace);
	}
}