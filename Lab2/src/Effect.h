#pragma once

#include <string>
#include "glincludes.h"
#include "Shader.h"

namespace zf
{
	class Effect
	{
	public:
		Effect();
		Effect(Effect &&);
		Effect & operator=(Effect &&);
		~Effect();

		GLuint ID() const { return programID; }

		Effect & Attach(const Shader &);
		void Link();
		void Apply();

		GLint Uniform(const char *name);

	private:
		Effect(const Effect &) {}
		Effect & operator=(const Effect &) {}

		void InitIfNeed();
		void Validate();

		GLuint programID;
		bool isCreated;
	};
}