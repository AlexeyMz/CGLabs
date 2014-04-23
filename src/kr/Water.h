#pragma once

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "../common/Mesh.h"

namespace zf
{
	class Water
	{
	public:
		Water();
		void Init();

		Texture2D & Texture() { return texturesABFlipped ? textureB : textureA; }

		void AddDrop(float x, float y, float radius, float strength);
		void MoveSphere(glm::vec3 oldCenter, glm::vec3 newCenter, float radius);
		void StepSimulation();
		void UpdateNormals();

	private:
		Water(const Water &) {}
		Water & operator=(const Water &) {}

		Texture2D & BackTexture() { return texturesABFlipped ? textureA : textureB; }
		
		void FlipTexturesAB()
		{
			texturesABFlipped = !texturesABFlipped;
		}

		Effect dropEffect, updateEffect, normalEffect, sphereEffect;
		Texture2D textureA, textureB;
		bool texturesABFlipped;

		Mesh plane;
	};
}