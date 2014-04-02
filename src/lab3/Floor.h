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

	private:
		Floor(const Floor &) {}
		Floor & operator=(const Floor &) {}

		Effect effect;
		Texture2D texture;

		GLint mvpUniform, ambientUniform, textureUniform;
	};
}