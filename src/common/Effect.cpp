#include "Effect.h"

#include <assert.h>

namespace zf
{
	Effect::Effect()
		: programID(0), isCreated(false)
	{
	}

	Effect::Effect(Effect &&fx)
		: programID(0), isCreated(false)
	{
		*this = std::move(fx);
	}

	Effect & Effect::operator=(Effect &&fx)
	{
		this->~Effect();
		programID = fx.programID;
		isCreated = fx.isCreated;
		fx.programID = 0;
		return *this;
	}

	Effect::~Effect()
	{
		if (programID) {
			glDeleteProgram(programID);
			programID = 0;
		}
	}

	Effect & Effect::Attach(const Shader &shader)
	{
		InitIfNeed();
		assert(shader.ID() != 0);
		glAttachShader(programID, shader.ID());
		return *this;
	}

	void Effect::Link()
	{
		assert(programID != 0);
		glLinkProgram(programID);
		Validate();
	}

	void Effect::Apply()
	{
		assert(programID != 0);
		glUseProgram(programID);
	}

	GLint Effect::Uniform(const char *name)
	{
		assert(programID != 0);
		return glGetUniformLocation(programID, name);
	}

	void Effect::InitIfNeed()
	{
		if (!isCreated && !programID) {
			programID = glCreateProgram();
			isCreated = true;
			if (!programID)
				throw ShaderLoadException("Could not create program object.");
		}
	}

	void Effect::Validate()
	{
		assert(programID != 0);
		GLint linked;
		glGetProgramiv(programID, GL_LINK_STATUS, &linked);
		if (!linked) {
			GLint maxLogLength = 0;
			glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLogLength);
			if (maxLogLength > 0) {
				GLsizei logLength = 0;
				GLchar *compilerLog = new char[maxLogLength];
				glGetProgramInfoLog(programID, maxLogLength, &logLength, compilerLog);
				auto message = std::string("Program link error: \n") + compilerLog;
				delete[] compilerLog;
				throw ShaderLoadException(message);
			} else {
				throw ShaderLoadException("Unknown program link error.");
			}
		}
	}
}