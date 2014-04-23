#include "Texture2D.h"

#include <fstream>
#include <vector>
#include <string>
#include <assert.h>
#include "TextureLoadException.h"
#include "utils.h"

namespace zf
{
	Texture2D::Texture2D()
		: id(0), width(0), height(0), components(0), format(0)
	{
	}

	Texture2D::~Texture2D()
	{
		if (glIsTexture(id))
			glDeleteTextures(1, &id);
		if (glIsRenderbuffer(renderBufferID))
			glDeleteRenderbuffers(1, &renderBufferID);
		if (glIsFramebuffer(frameBufferID))
			glDeleteFramebuffers(1, &frameBufferID);
	}

	void Texture2D::Bind()
	{
		Bind(0);
	}

	void Texture2D::Bind(GLuint index)
	{
		assert(id != 0);
		glActiveTexture(GL_TEXTURE0 + index);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void Texture2D::AllocateTextureObject()
	{
		assert(id == 0);
		glGenTextures(1, &id);
		if (id <= 0)
			throw new TextureLoadException("Failed to allocate texture object.");
		Bind();
	}

	void Texture2D::LoadFromFile(const char *fileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode)
	{
		auto pixelData = ReadTGABits(fileName, width, height, components, format);

		AllocateTextureObject();
		
		// set wrapping and filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		GLfloat largestAnisotropy;
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestAnisotropy);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largestAnisotropy);

		// load pixel data into server memory
		glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, &pixelData[0]);

		if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter == GL_LINEAR_MIPMAP_NEAREST ||
			minFilter == GL_NEAREST_MIPMAP_LINEAR || minFilter == GL_NEAREST_MIPMAP_NEAREST)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	void Texture2D::InitializeEmpty(GLint width, GLint height, GLenum minFilter, GLenum magFilter, GLenum type)
	{
		AllocateTextureObject();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		this->width = width;
		this->height = height;
		this->components = type == GL_FLOAT ? GL_RGBA32F : GL_RGBA;
		
		std::vector<char> data;
		data.resize(width * height * 4 * (type == GL_FLOAT ? 4 : 1));
		glTexImage2D(GL_TEXTURE_2D, 0, this->components, width, height, 0, GL_RGBA, type, &data[0]);
	}

	void Texture2D::AllocateFrameAndRenderBuffers()
	{
		assert(id != 0);
		assert(frameBufferID == 0);
		assert(renderBufferID == 0);

		glGenFramebuffers(1, &frameBufferID);
		if (frameBufferID <= 0)
			throw new TextureLoadException("Failed to allocate frame buffer object.");
		glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);

		glGenRenderbuffers(1, &renderBufferID);
		if (renderBufferID <= 0)
			throw new TextureLoadException("Failed to allocate render buffer object.");
		glBindRenderbuffer(GL_RENDERBUFFER, renderBufferID);

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
	}
}