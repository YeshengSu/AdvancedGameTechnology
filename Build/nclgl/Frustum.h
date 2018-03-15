#pragma once

#include "Plane.h"
#include "Matrix4.h"
#include "RenderNode.h"
class Matrix4; // Compile the Mat4 class first , please !

class RenderNode;
class Frustum {
public:
	Frustum(void) {};
	~Frustum(void) {};
	
	void FromMatrix(const Matrix4 & mvp);
	bool InsideFrustum(RenderNode &n);

	Plane& GetPlane(int i) {
		return planes[i];
	}

protected:
	Plane planes[6];
};

