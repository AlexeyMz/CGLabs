#include "Water.h"

#include "../common/CommonDeclarations.h"

namespace zf
{
	Water::Water()
		: texturesABFlipped(false)
	{
	}

	void Water::Init()
	{
		plane.AddPlane(1, 1);

		Shader drop(GL_FRAGMENT_SHADER);
		Shader update(GL_FRAGMENT_SHADER);
		Shader normal(GL_FRAGMENT_SHADER);
		Shader sphere(GL_FRAGMENT_SHADER);

		Shader vertex(GL_VERTEX_SHADER);
		vertex.CompileFile("fx/water_vertex.glsl");

		dropEffect.Attach(vertex).Attach(drop.CompileFile("fx/water_drop.glsl")).Link();
		updateEffect.Attach(vertex).Attach(update.CompileFile("fx/water_update.glsl")).Link();
		normalEffect.Attach(vertex).Attach(normal.CompileFile("fx/water_normal.glsl")).Link();
		sphereEffect.Attach(vertex).Attach(sphere.CompileFile("fx/water_sphere.glsl")).Link();

		const GLint width = 256;
		const GLint height = 256;
		textureA.InitializeEmpty(width, height, GL_LINEAR, GL_LINEAR, GL_FLOAT);
		textureB.InitializeEmpty(width, height, GL_LINEAR, GL_LINEAR, GL_FLOAT);
	}

	void Water::AddDrop(float x, float y, float radius, float strength)
	{
		SetRenderTo renderTo(BackTexture());
		Texture().Bind();
		
		zf::Effect &eff = dropEffect;
		eff.Apply();
		glUniform2f(eff.Uniform("center"), x, y);
		glUniform1f(eff.Uniform("radius"), radius);
		glUniform1f(eff.Uniform("strength"), strength);

		plane.Draw();

		FlipTexturesAB();
	}

	void Water::MoveSphere(glm::vec3 oldCenter, glm::vec3 newCenter, float radius)
	{
		SetRenderTo renderTo(BackTexture());
		Texture().Bind();
		
		zf::Effect &eff = sphereEffect;
		eff.Apply();
		glUniform3fv(eff.Uniform("oldCenter"), 1, glm::value_ptr(oldCenter));
		glUniform3fv(eff.Uniform("newCenter"), 1, glm::value_ptr(newCenter));
		glUniform1f(eff.Uniform("radius"), radius);

		plane.Draw();

		FlipTexturesAB();
	}

	void Water::StepSimulation()
	{
		SetRenderTo renderTo(BackTexture());
		Texture().Bind();
		
		zf::Effect &eff = updateEffect;
		eff.Apply();
		glUniform2f(eff.Uniform("delta"), 1.0f / Texture().Width(), 1.0f / Texture().Height());

		plane.Draw();

		FlipTexturesAB();
	}

	void Water::UpdateNormals()
	{
		SetRenderTo renderTo(BackTexture());
		Texture().Bind();

		zf::Effect &eff = normalEffect;
		eff.Apply();
		glUniform2f(eff.Uniform("delta"), 1.0f / Texture().Width(), 1.0f / Texture().Height());

		plane.Draw();

		FlipTexturesAB();
	}
}