#pragma once

#include "../common/glincludes.h"
#include "../common/Effect.h"
#include "../common/Texture2D.h"
#include "CommonDeclarations.h"

namespace zf
{
	class Floor
	{
	public:
		Floor();
		~Floor();

		void Init();
		void Draw(glm::mat4 mvp);

		float Height;
		float Size;
		Texture2D Texture;

	private:
		Floor(const Floor &) {}
		Floor & operator=(const Floor &) {}

		Effect effect;
		GLint mvpUniform, ambientUniform, textureUniform;
	};
}