#include "Floor.h"

namespace zf
{
	Floor::Floor()
		: Height(-1)
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

		texture.LoadFromFile("textures/floor.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_MIRRORED_REPEAT);
	}

	void Floor::Draw(glm::mat4 viewProjection)
	{
		glm::vec3 points[] = {
			glm::vec3(-1, 0, -1),  // 0----1   o--> x
			glm::vec3( 1, 0, -1),  // |    |   |
			glm::vec3( 1, 0,  1),  // |    |   v
			glm::vec3(-1, 0,  1),  // 3----2   z
		};
		auto up = glm::vec3(0, 1, 0);
		glm::vec3 normals[] = { up, up, up, up };
		float repeatCount = 30;
		float texCoords[] = {
			0, repeatCount,
			repeatCount, repeatCount,
			repeatCount, 0,
			0, 0,
		};
		GLubyte indices[] = {
			0, 3, 1,
			3, 2, 1,
		};

		effect.Apply();

		float size = 100;
		auto mvp = viewProjection * glm::mat4(
			size, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, size, 0,
			0, Height, 0, 1);
		glUniformMatrix4fv(mvpUniform, 1, false, glm::value_ptr(mvp));
		glUniform1f(ambientUniform, 1);
		glBindTexture(GL_TEXTURE_2D, texture.ID());

		UseVertexAttribPointer position(VertexAttr::POSITION);
		UseVertexAttribPointer texCoord0(VertexAttr::TEXCOORD0);
		glVertexAttribPointer(position, 3, GL_FLOAT, false, 0, points);
		glVertexAttribPointer(texCoord0, 2, GL_FLOAT, false, 0, texCoords);
		glDrawElements(GL_TRIANGLES, 2 * 3, GL_UNSIGNED_BYTE, indices);
	}
}