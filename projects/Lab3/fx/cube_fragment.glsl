#version 330

uniform mat3 MV3x3;
uniform sampler2D TextureUnit0;
uniform sampler2D NormalSampler;
uniform float Ambient;

in vec2 FragTexCoord;
in vec3 LightDirection_cameraspace;
in vec3 vNormal_modelspace;
in vec3 vTangent_modelspace;
in vec3 vBitangent_modelspace;

layout(location = 0) out vec4 FragmentColor;

void main()
{
	vec3 vertexNormal_cameraspace = MV3x3 * normalize(vNormal_modelspace);
	vec3 vertexTangent_cameraspace = MV3x3 * normalize(vTangent_modelspace);
	vec3 vertexBitangent_cameraspace = MV3x3 * normalize(vBitangent_modelspace);

	mat3 TBN = transpose(mat3(
        vertexTangent_cameraspace,
        vertexBitangent_cameraspace,
        vertexNormal_cameraspace
    ));

	vec3 lightDirection_tangentspace = TBN * LightDirection_cameraspace;
	// Local normal, in tangent space
	vec3 textureNormal_tangentspace = normalize(texture2D(NormalSampler, FragTexCoord).rgb * 2.0 - 1.0);
	
	/*
	// Normal of the computed fragment, in camera space
	vec3 n = normalize(Normal_cameraspace);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(LightDirection_cameraspace);
	*/
	vec3 n = textureNormal_tangentspace;
	vec3 l = lightDirection_tangentspace;

	// Cosine of the angle between the normal and the light direction,
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp(dot(n, l), 0, 1);

	FragmentColor = texture2D(TextureUnit0, FragTexCoord) * (cosTheta + Ambient * vec4(1));
}