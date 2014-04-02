#pragma once

#include "glincludes.h"

namespace zf
{
	class Texture2D
	{
	public:
		Texture2D();
		~Texture2D();

		GLuint ID() const { return id; }
		GLint Width() const { return width; }
		GLint Height() const { return height; }
		GLenum Format() const { return format; }

		void LoadFromFile(const char *fileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);

	private:
		Texture2D(const Texture2D &) {}
		Texture2D & operator=(const Texture2D &) {}

		GLuint id;
		GLint width, height, components;
		GLenum format;
	};
}