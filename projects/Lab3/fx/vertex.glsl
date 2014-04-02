#version 330

uniform mat4 mWorld;
uniform mat4 mView;
uniform mat4 mProjection;
uniform vec3 vLightPosition;
uniform vec4 vLightColor;
uniform float fAmbient;

layout(location = 0) in vec3 vPosition;
layout(location = 2) in vec3 vNormal;
layout(location = 3) in vec2 vTexCoord0;
out vec4 vFColor;
out vec2 vFTex;

void main()
{
	mat3 mNormalMatrix;
	mNormalMatrix[0] = normalize(mWorld[0].xyz);
	mNormalMatrix[1] = normalize(mWorld[1].xyz);
	mNormalMatrix[2] = normalize(mWorld[2].xyz);
	vec3 vNorm = normalize(mNormalMatrix * vNormal);

    vec3 vLightDir = normalize(vLightPosition - vPosition);
    float fDot = max(0.0, dot(vNorm, vLightDir));
    vFColor.rgb = vLightColor.rgb * min(1.0, fDot + fAmbient);
    vFColor.a = vLightColor.a;
    vFTex = vTexCoord0;

    mat4 mvpMatrix = mProjection * mView * mWorld;
    gl_Position = mvpMatrix * vec4(vPosition, 1);
}