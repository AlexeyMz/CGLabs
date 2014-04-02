#pragma once

#include <stdexcept>

namespace zf
{
	class TextureLoadException : public std::runtime_error
	{
	public:
		explicit TextureLoadException(std::string message)
			: std::runtime_error(message)
		{
		}
	};
}