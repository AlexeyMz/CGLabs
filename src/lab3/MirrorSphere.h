#pragma once

#include <vector>
#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/CommonDeclarations.h"

namespace zf
{
	class MirrorSphere
	{
	public:
		MirrorSphere();
		~MirrorSphere();

		void Init();
		void Draw(
			glm::mat4 model, glm::mat4 view, glm::mat4 projection,
			Texture2D &floorTexture, glm::mat4 floorModel);

		float Height;

	private:
		MirrorSphere(const MirrorSphere &) {}
		MirrorSphere & operator=(const MirrorSphere &) {}

		void GenerateGeosphere(int level);

		Effect effect;

		std::vector<glm::vec3> points;

		GLint modelUniform, viewUniform, projectionUniform;
		GLint floorTextureUniform, floorModelUniform;
	};
}