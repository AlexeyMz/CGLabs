#include "Floor.h"

namespace zf
{
	Floor::Floor()
		: Height(-1), Size(5)
	{
	}

	Floor::~Floor()
	{
	}

	void Floor::Init()
	{
		zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
		effect.Attach(vertex.CompileFile("fx/floor_vertex.glsl"))
			.Attach(fragment.CompileFile("fx/floor_fragment.glsl"))
			.Link();

		mvpUniform = effect.Uniform("mMVP");
		ambientUniform = effect.Uniform("fAmbient");
		textureUniform = effect.Uniform("textureUnit0");

		Texture.LoadFromFile("textures/baku.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT);
	}

	void Floor::Draw(glm::mat4 modelViewProjection)
	{
		glm::vec3 points[] = {
			glm::vec3(-1, 0, -1),  // 0----1   o--> x
			glm::vec3( 1, 0, -1),  // |    |   |
			glm::vec3( 1, 0,  1),  // |    |   v
			glm::vec3(-1, 0,  1),  // 3----2   z
		};
		auto up = glm::vec3(0, 1, 0);
		glm::vec3 normals[] = { up, up, up, up };
		float repeatCount = 1;
		float texCoords[] = {
			0, 0,
			0, repeatCount,
			repeatCount, repeatCount,
			repeatCount, 0,
		};
		GLubyte indices[] = {
			0, 3, 1,
			3, 2, 1,
		};

		effect.Apply();
		glUniformMatrix4fv(mvpUniform, 1, false, glm::value_ptr(modelViewProjection));
		glUniform1f(ambientUniform, 1);
		Texture.Bind();
		glUniform1i(textureUniform, 0);

		UseVertexAttribPointer position(VertexAttr::POSITION, 3, GL_FLOAT, false, 0, points);
		UseVertexAttribPointer texCoord0(VertexAttr::TEXCOORD0, 2, GL_FLOAT, false, 0, texCoords);
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_BYTE, indices);
	}
}