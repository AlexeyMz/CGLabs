#pragma once

#include <string>

#include "glincludes.h"

namespace zf
{
	struct BindCubemap;

	class Cubemap
	{
	public:
		friend struct BindCubemap;

		Cubemap();
		~Cubemap();

		// pattern example: "fx/cubemap/file_*.tga"
		//   * replaced by ['xpos', 'xneg', 'ypos', 'yneg', 'zpos', 'zneg']
		void LoadFromFiles(const std::string &pattern);

	private:
		Cubemap(const Cubemap &) {}
		Cubemap & operator=(const Cubemap &) {}

		GLuint id;
	};

	struct BindCubemap
	{
		BindCubemap(const Cubemap &cubemap, GLuint index)
			: textureIndex(index)
		{
			assert(cubemap.id != 0);
			glActiveTexture(GL_TEXTURE0 + textureIndex);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap.id);
		}

		~BindCubemap()
		{
			glActiveTexture(GL_TEXTURE0 + textureIndex);
			glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		}

	private:
		BindCubemap(const BindCubemap &) {}
		BindCubemap & operator=(const BindCubemap &) {}

		GLuint textureIndex;
	};
}