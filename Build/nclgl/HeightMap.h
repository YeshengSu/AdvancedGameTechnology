#pragma once
#include <string>
#include <iostream>
#include <fstream>

#include "..\nclgl\mesh.h"


#ifndef HEIGHT_MAP_DEF_OFF
#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f/16.0f
#define HEIGHTMAP_TEX_Z 1.0f/16.0f
#endif

class HeightMap : public Mesh {
public:
	HeightMap(std::string name, const uint rawWidth = RAW_WIDTH, const uint rawHeight = RAW_HEIGHT, const float HeightMapX= HEIGHTMAP_X, const float HeightMapY = HEIGHTMAP_Y, const float HeightMapZ = HEIGHTMAP_Z, const float HeightMapTexX = HEIGHTMAP_TEX_X, const float HeightMapTexZ = HEIGHTMAP_TEX_Z);
	~HeightMap(void) {};

};
