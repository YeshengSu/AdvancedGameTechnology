#pragma once
#include "CudaCommon.cuh"
#include <nclgl\common.h>



class CudaParticles
{
public:
	CudaParticles();
	~CudaParticles();

	void InitializeArrays(uint size, GLuint glVertexBuffer);
	void UpdateParticles(uint num_verts_to_update, float dt);

protected:
	uint num_particles;

	//Pointer to our GPU array, this is an indirect pointer and cant be directly accessed via c++
	float3* cArrVelocities; 

	//This will be a reference to our opengl vertex buffer but could also just be a direct array like above
	cudaGraphicsResource* cGLPositions;
};