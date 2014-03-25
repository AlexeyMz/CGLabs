#version 330

uniform mat4 mView;
uniform mat4 mProjection;

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec4 vColor;
out vec4 vFColor;

void main()
{
	gl_Position =  mProjection * mView * vec4(vPosition, 1);
	vFColor = vColor;
}