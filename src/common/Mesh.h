#pragma once

#include <vector>

#include "glincludes.h"

namespace zf
{
	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		bool GenerateTextureCoords;
		bool GenerateNormals;

		std::vector<glm::vec3> Vertices;
		std::vector<glm::uvec3> Triangles;
		std::vector<glm::vec2> Coords;
		std::vector<glm::vec3> Normals;

		void AddPlane(int detailX, int detailY);
		void AddCube();

		void Draw();

	private:
		Mesh(const Mesh &) {}
		Mesh & operator=(const Mesh &) {}
	};
}