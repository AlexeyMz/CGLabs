#version 330

uniform mat4 mMVP;
uniform float fAmbient;

layout(location = 0) in vec3 vPosition;
layout(location = 3) in vec2 vTexCoord0;
out vec4 vFColor;
out vec2 vFTex;

void main()
{
    vFColor.rgb = vec3(1, 1, 1) * fAmbient;
    vFColor.a = 1;
    vFTex = vTexCoord0;

    gl_Position = mMVP * vec4(vPosition, 1);
}