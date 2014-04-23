#pragma once

#include "glincludes.h"
#include "TextureLoadException.h"

namespace zf
{
	class SetRenderTo;

	class Texture2D
	{
	public:
		friend class SetRenderTo;

		Texture2D();
		~Texture2D();

		GLuint ID() const { return id; }
		void Bind();
		void Bind(GLuint index);

		GLint Width() const { return width; }
		GLint Height() const { return height; }
		GLenum Format() const { return format; }

		void LoadFromFile(const char *fileName, GLenum minFilter, GLenum magFilter, GLenum wrapMode);
		void InitializeEmpty(GLint width, GLint height, GLenum minFilter, GLenum magFilter, GLenum type);

	private:
		Texture2D(const Texture2D &) {}
		Texture2D & operator=(const Texture2D &) {}

		void AllocateTextureObject();
		void AllocateFrameAndRenderBuffers();

		GLuint id;
		GLint width, height, components;
		GLenum format;

		GLuint frameBufferID, renderBufferID;
	};

	class SetRenderTo
	{
	public:
		SetRenderTo(Texture2D &texture)
		{
			glGetIntegerv(GL_VIEWPORT, viewport);

			if (texture.frameBufferID == 0) {
				texture.AllocateFrameAndRenderBuffers();
			} else {
				glBindFramebuffer(GL_FRAMEBUFFER, texture.frameBufferID);
				glBindRenderbuffer(GL_RENDERBUFFER, texture.renderBufferID);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texture.renderBufferID);

			GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

			/*GLuint clearColor[4] = {0, 0, 0, 0};
			glClearBufferuiv(GL_COLOR, 0, clearColor);
			glClearBufferuiv(GL_DEPTH, 0, &clearColor[0]);*/

			glViewport(0, 0, texture.width, texture.height);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				throw new TextureLoadException("Failed to create frame buffer.");

			//glClearColor(1, 0, 0, 1);
			//glClear(GL_COLOR_BUFFER_BIT);
		}

		~SetRenderTo()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
			glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
		}

	private:
		SetRenderTo(const SetRenderTo &) {}
		SetRenderTo & operator=(const SetRenderTo &) {}

		GLint viewport[4];
	};
}