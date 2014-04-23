#include "Cubemap.h"

#include "utils.h"
#include "TextureLoadException.h"

namespace zf
{
	Cubemap::Cubemap()
		: id(0)
	{
	}

	Cubemap::~Cubemap()
	{
		if (glIsTexture(id))
			glDeleteTextures(1, &id);
	}

	void Cubemap::LoadFromFiles(const std::string &pattern)
	{
		assert(id == 0);

		glGenTextures(1, &id);
		if (id <= 0)
			throw new TextureLoadException("Failed to allocate texture object for cubemap.");

		std::string files[] = {
			pattern,
			pattern,
			pattern,
			pattern,
			pattern,
			pattern,
		};
		replace(files[0], "*", "xneg");
		replace(files[1], "*", "xpos");
		replace(files[2], "*", "yneg");
		replace(files[3], "*", "ypos");
		replace(files[4], "*", "zneg");
		replace(files[5], "*", "zpos");

		GLint width[6], height[6], components[6];
		GLenum format[6];

		auto dataXNeg = ReadTGABits(files[0].c_str(), width[0], height[0], components[0], format[0]);
		auto dataXPos = ReadTGABits(files[1].c_str(), width[1], height[1], components[1], format[1]);
		auto dataYNeg = ReadTGABits(files[2].c_str(), width[2], height[2], components[2], format[2]);
		auto dataYPos = ReadTGABits(files[3].c_str(), width[3], height[3], components[3], format[3]);
		auto dataZNeg = ReadTGABits(files[4].c_str(), width[4], height[4], components[4], format[4]);
		auto dataZPos = ReadTGABits(files[5].c_str(), width[5], height[5], components[5], format[5]);

		glBindTexture(GL_TEXTURE_CUBE_MAP, id);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, components[0], width[0], height[0], 0, format[0], GL_UNSIGNED_BYTE, &dataXNeg[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, components[1], width[1], height[1], 0, format[1], GL_UNSIGNED_BYTE, &dataXPos[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, components[2], width[2], height[2], 0, format[2], GL_UNSIGNED_BYTE, &dataYNeg[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, components[3], width[3], height[3], 0, format[3], GL_UNSIGNED_BYTE, &dataYPos[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, components[4], width[4], height[4], 0, format[4], GL_UNSIGNED_BYTE, &dataZNeg[0]);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, components[5], width[5], height[5], 0, format[5], GL_UNSIGNED_BYTE, &dataZPos[0]);
	}
}