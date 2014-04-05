#pragma once

#include <stdexcept>

namespace zf
{
	class ShaderLoadException : public std::runtime_error
	{
	public:
		explicit ShaderLoadException(std::string message)
			: std::runtime_error(message)
		{
		}

		explicit ShaderLoadException(std::string message, std::string shaderPath)
			: std::runtime_error(message), shaderPath(shaderPath)
		{
		}

		const std::string ShaderPath() const { return shaderPath; }

	private:
		std::string shaderPath;
	};
}