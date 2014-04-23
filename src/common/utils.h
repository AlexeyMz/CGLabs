#pragma once

#include <vector>
#include <string>
#include "glincludes.h"

namespace zf
{
	std::string format(const char *fmt, ...);
	std::string vformat(const char *fmt, va_list ap);
	bool ReadAllBytes(char const* filename, std::vector<char> &result);
	std::string ToString(const glm::vec3 &v);
	std::string ToString(const glm::vec4 &v);
	bool replace(std::string& str, const std::string& from, const std::string& to);
	std::vector<GLbyte> ReadTGABits(const char *fileName, GLint &width, GLint &height, GLint &components, GLenum &format);
}