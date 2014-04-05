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
}