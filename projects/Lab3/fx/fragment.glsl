#version 330

uniform sampler2D textureUnit0;

in vec4 vFColor;
in vec2 vFTex;
layout(location = 0) out vec4 fragmentColor;

void main()
{
	fragmentColor = vFColor * texture2D(textureUnit0, vFTex);
}