#include "Shader.h"

#include <vector>
#include <assert.h>

#include "utils.h"

namespace zf
{
	Shader::Shader(GLenum type)
		: id(0), type(type)
	{
	}

	Shader::Shader(Shader &&s)
		: id(0), type(0)
	{
		*this = std::move(s);
	}

	Shader & Shader::operator=(Shader &&s)
	{
		this->~Shader();
		id = s.id;
		s.id = 0;
		return *this;
	}

	Shader::~Shader()
	{
		if (id) {
			glDeleteShader(id);
			id = 0;
		}
	}

	Shader & Shader::CompileText(const char *source)
	{
		assert(id == 0);
		id = glCreateShader(type);
		if (!id)
			throw ShaderLoadException("Could not create shader object.");

		const GLchar *shaderSource = source;
		GLint shaderLength = std::strlen(source);
		glShaderSource(id, 1, &shaderSource, &shaderLength);

		glCompileShaderARB(id);
		Validate();

		return *this;
	}

	Shader & Shader::CompileFile(const char *fileName)
	{
		std::vector<char> bytes;
		if (!ReadAllBytes(fileName, bytes))
			throw ShaderLoadException("Cannot read shader file.", fileName);

		try {
			bytes.push_back(0);
			return CompileText(&bytes[0]);
		} catch (ShaderLoadException &ex) {
			throw ShaderLoadException(ex.what(), fileName);
		}
	}

	void Shader::Validate()
	{
		assert(id != 0);
		GLint compileResult;
		glGetShaderiv(id, GL_COMPILE_STATUS, &compileResult);
		if (!compileResult) {
			GLint maxLogLength = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLogLength);
			if (maxLogLength > 0) {
				GLsizei logLength = 0;
				char *compilerLog = new char[maxLogLength];
				glGetShaderInfoLog(id, maxLogLength, &logLength, compilerLog);
				auto message = std::string("Shader compile error: \n") + compilerLog;
				delete[] compilerLog;
				throw ShaderLoadException(message);
			} else {
				throw ShaderLoadException("Unknown shader compile error.");
			}
		}
	}
}