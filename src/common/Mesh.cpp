#include "Mesh.h"

#include "CommonDeclarations.h"

namespace zf
{
	Mesh::Mesh()
		: GenerateTextureCoords(false), GenerateNormals(false)
	{
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::AddPlane(int detailX, int detailY)
	{
		for (int y = 0; y <= detailY; y++) {
			float t = 1.0f * y / detailY;
			for (int x = 0; x <= detailX; x++) {
				float s = 1.0f * x / detailX;
				Vertices.push_back(glm::vec3(2 * s - 1, 2 * t - 1, 0));
				if (GenerateTextureCoords) Coords.push_back(glm::vec2(s, t));
				if (GenerateNormals) Normals.push_back(glm::vec3(0, 0, 1));
				if (x < detailX && y < detailY) {
					int i = x + y * (detailX + 1);
					Triangles.push_back(glm::uvec3(i, i + 1, i + detailX + 1));
					Triangles.push_back(glm::uvec3(i + detailX + 1, i + 1, i + detailX + 2));
				}
			}
		}
	}

	namespace
	{
		int const cubeData[][7] = {
			{0, 4, 2, 6, -1, 0, 0}, // -x
			{1, 3, 5, 7, +1, 0, 0}, // +x
			{0, 1, 4, 5, 0, -1, 0}, // -y
			{2, 6, 3, 7, 0, +1, 0}, // +y
			{0, 2, 1, 3, 0, 0, -1}, // -z
			{4, 5, 6, 7, 0, 0, +1}, // +z
		};

		glm::ivec3 pickOctant(int i)
		{
			return glm::ivec3((i & 1) * 2 - 1, (i & 2) - 1, (i & 4) / 2 - 1);
		}
	};

	void Mesh::AddCube()
	{
		for (int i = 0; i < 6; i++) {
			int v = i * 4;
			for (int j = 0; j < 4; j++) {
				int d = cubeData[i][j];
				glm::ivec3 oct = pickOctant(d);
				Vertices.push_back(glm::vec3(oct.x, oct.y, oct.z));
				if (GenerateTextureCoords) Coords.push_back(glm::vec2(j & 1, (j & 2) / 2));
				if (GenerateNormals) Normals.push_back(glm::vec3(cubeData[i][4], cubeData[i][5], cubeData[i][6]));
			}
			Triangles.push_back(glm::uvec3(v, v + 1, v + 2));
			Triangles.push_back(glm::uvec3(v + 2, v + 1, v + 3));
		}
	};

	void Mesh::Draw()
	{
		UseVertexAttribPointer position(VertexAttr::POSITION, 3, GL_FLOAT, false, 0, &Vertices[0]);
		//UseVertexAttribPointer texCoord0(VertexAttr::TEXCOORD0, 2, GL_FLOAT, false, 0, &Coords[0]);
		glDrawElements(GL_TRIANGLES, 3 * Triangles.size(), GL_UNSIGNED_INT, &Triangles[0]);
	}
}