#version 330

in vec4 vFColor;
layout(location = 0) out vec4 fragmentColor;

void main()
{
	fragmentColor = vFColor;
}