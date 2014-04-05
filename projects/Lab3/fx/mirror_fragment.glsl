#version 330

uniform mat4 View;
uniform sampler2D FloorTexture;
uniform mat4 FloorModel;

smooth in vec3 Position_worldspace;
smooth in vec3 Position_cameraspace;
smooth in vec3 Normal_cameraspace;

layout(location = 0) out vec4 fragmentColor;

void main()
{
	vec3 incidentEye = normalize(Position_cameraspace);
	vec3 normal = normalize(Normal_cameraspace);
	vec3 reflected_cameraspace = reflect(incidentEye, normal);
	vec3 reflected_worldspace = normalize(vec3(inverse(View) * vec4(reflected_cameraspace, 0)));

	vec3 floorPosition = vec3(FloorModel[3]);
	mat3 floorInvScale = inverse(mat3(FloorModel));

	float distance = Position_worldspace.y - floorPosition.y;
	if (sign(distance) == sign(reflected_worldspace.y))
		fragmentColor = vec4(0, 0, 0, 1);
	else {
		vec2 texPos_worldspace = (distance / reflected_worldspace.y) * reflected_worldspace.xz + Position_worldspace.xz - floorPosition.xz;
		vec2 texPos_texspace = (floorInvScale * vec3(texPos_worldspace.x, 0, texPos_worldspace.y)).xz;
		texPos_texspace = (1 - vec2(texPos_texspace.y, texPos_texspace.x)) * 0.5; // map to [0..1]
		if (texPos_texspace.x < 0 || texPos_texspace.x > 1 || texPos_texspace.y < 0 || texPos_texspace.y > 1)
			fragmentColor = vec4(0, 0, 0, 1);
		else
			fragmentColor = texture2D(FloorTexture, texPos_texspace);
	}
}