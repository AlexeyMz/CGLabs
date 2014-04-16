#pragma once

namespace zf
{
	namespace VertexAttr {
		enum VertexAttr {
			POSITION = 0,
			COLOR = 1,
			NORMAL = 2,
			TEXCOORD0 = 3,
			TANGENT = 4,
			BITANGENT = 5,
		};
	}

	struct UseVertexAttribPointer
	{
		UseVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
			: attributeIndex(index)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		}

		~UseVertexAttribPointer()
		{
			glDisableVertexAttribArray(attributeIndex);
		}

		operator GLuint() { return attributeIndex; }

		GLuint attributeIndex;
	};

	struct UseCapability
	{
		UseCapability(GLenum cap, bool enable)
			: cap(cap)
		{
			previousState = glIsEnabled(cap);
			if (enable)
				glEnable(cap);
			else
				glDisable(cap);
		}

		~UseCapability()
		{
			if (previousState)
				glEnable(cap);
			else
				glDisable(cap);
		}

		GLenum cap;
		GLboolean previousState;
	};
}