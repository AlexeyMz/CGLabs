#pragma once

#include <stdexcept>

namespace zf {
	class ShaderLoadException : public std::runtime_error
	{
	public:
		explicit ShaderLoadException(std::string message)
			: std::runtime_error(message)
		{
		}
	};
}