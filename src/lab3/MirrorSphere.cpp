#include "MirrorSphere.h"

namespace zf
{
	MirrorSphere::MirrorSphere()
	{
	}

	MirrorSphere::~MirrorSphere()
	{
	}

	void MirrorSphere::Init()
	{
		zf::Shader vertex(GL_VERTEX_SHADER), fragment(GL_FRAGMENT_SHADER);
		effect.Attach(vertex.CompileFile("fx/mirror_vertex.glsl"))
			.Attach(fragment.CompileFile("fx/mirror_fragment.glsl"))
			.Link();

		modelUniform = effect.Uniform("Model");
		viewUniform = effect.Uniform("View");
		projectionUniform = effect.Uniform("Projection");
		floorTextureUniform = effect.Uniform("FloorTexture");
		floorTextureScalingUniform = effect.Uniform("FloorTextureScaling");
		floorHeightUniform = effect.Uniform("FloorHeight");

		GenerateGeosphere(4);
	}

	void MirrorSphere::Draw(
		glm::mat4 model, glm::mat4 view, glm::mat4 projection,
		Texture2D &floorTexture, float floorTextureScaling, float floorHeight)
	{
		effect.Apply();
		glUniformMatrix4fv(modelUniform, 1, false, glm::value_ptr(model));
		glUniformMatrix4fv(viewUniform, 1, false, glm::value_ptr(view));
		glUniformMatrix4fv(projectionUniform, 1, false, glm::value_ptr(projection));
		glUniform1f(floorTextureScalingUniform, floorTextureScaling);
		glUniform1f(floorHeightUniform, floorHeight);
		glBindTexture(GL_TEXTURE_2D, floorTexture.ID());
		glUniform1i(floorTextureUniform, 0);

		UseVertexAttribPointer position(VertexAttr::POSITION);
		UseVertexAttribPointer normal(VertexAttr::NORMAL);
		glVertexAttribPointer(position, 3, GL_FLOAT, false, 0, &points[0]);
		glVertexAttribPointer(normal, 3, GL_FLOAT, false, 0, &points[0]);
		glDrawArrays(GL_TRIANGLES, 0, points.size());
	}

	void MirrorSphere::GenerateGeosphere(int level)
	{
		glm::vec3 start[] = {
			glm::vec3(0, 1, 0),
			glm::normalize(glm::vec3(0, -1, -1)),
			glm::normalize(glm::vec3(-1, 0, 1)),
			glm::normalize(glm::vec3(1, 0, 1)),
		};

		std::vector<glm::vec3> p;
		p.push_back(start[0]);
		p.push_back(start[1]);
		p.push_back(start[2]);

		p.push_back(start[0]);
		p.push_back(start[2]);
		p.push_back(start[3]);

		p.push_back(start[0]);
		p.push_back(start[3]);
		p.push_back(start[1]);

		p.push_back(start[1]);
		p.push_back(start[3]);
		p.push_back(start[2]);

		for (int i = 0; i < level; i++) {
			for (int j = 0; j < p.size(); j += 3) {
				auto p1 = p[j + 0];
				auto p2 = p[j + 1];
				auto p3 = p[j + 2];
				auto p12 = glm::normalize((p1 + p2) * .5f);
				auto p23 = glm::normalize((p2 + p3) * .5f);
				auto p31 = glm::normalize((p3 + p1) * .5f);
				
				points.push_back(p1);
				points.push_back(p12);
				points.push_back(p31);

				points.push_back(p2);
				points.push_back(p23);
				points.push_back(p12);

				points.push_back(p3);
				points.push_back(p31);
				points.push_back(p23);

				points.push_back(p12);
				points.push_back(p23);
				points.push_back(p31);
			}

			if (i != level - 1) {
				p = points;
				points.clear();
			}
		}
	}
}