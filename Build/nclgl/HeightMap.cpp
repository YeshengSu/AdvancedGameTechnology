#include "HeightMap.h"

HeightMap::HeightMap(std::string name, const uint rawWidth, const uint rawHeight, const float HeightMapX, const float HeightMapY, const float HeightMapZ, const float HeightMapTexX, const float HeightMapTexZ) {
	std::ifstream file(name.c_str(), ios::binary);
	if (!file) {
		return;
	}
	numVertices = rawWidth * rawHeight;
	numIndices = (rawWidth - 1)*(rawHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];
	unsigned char * data = new unsigned char[numVertices];
	file.read((char *)data, numVertices * sizeof(unsigned char));
	file.close();

	for (uint x = 0; x < rawWidth; ++x) {
		for (uint z = 0; z < rawHeight; ++z) {
			uint offset = (x * rawWidth) + z;

			vertices[offset] = Vector3(x * HeightMapX, data[offset] * HeightMapY, z * HeightMapZ);

			textureCoords[offset] = Vector2(x * HeightMapTexX, z * HeightMapTexZ);
		}
	}
	delete data;
	numIndices = 0;

	for (uint x = 0; x < rawWidth - 1; ++x) {
		for (uint z = 0; z < rawHeight - 1; ++z) {
			uint a = (x * (rawWidth)) + z;
			uint b = ((x + 1) * (rawWidth)) + z;
			uint c = ((x + 1) * (rawWidth)) + (z + 1);
			uint d = (x * (rawWidth)) + (z + 1);

			indices[numIndices++] = c;
			indices[numIndices++] = b;
			indices[numIndices++] = a;

			indices[numIndices++] = a;
			indices[numIndices++] = d;
			indices[numIndices++] = c;
		}
	}
	GenerateNormals();
	GenerateTangents();

	BufferData();
}
