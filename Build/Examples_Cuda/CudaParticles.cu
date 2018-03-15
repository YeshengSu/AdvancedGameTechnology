#include "CudaParticles.cuh"
#include <random>


//This is run on the GPU, you can write (almost) c++ code and functions that
// run on the graphics card. These helper functions will get compiled both on the 
// CPU (__host__ tag) and GPU (__device__ tag) so can be accessed both inside
// kernals and in our program.
__host__ __device__ void initialize_y_vel(float3& vel)
{
	float vel_xy_sq = vel.x * vel.x + vel.z * vel.z;
	//Just a random func, to generate a nice(?) fountain effect for our particles
	vel.y = (7.5f - vel_xy_sq * vel_xy_sq) * 2.f;
	//vel.y = (15.f - vel_xy_sq); ///standard 'curve'
}
__host__ __device__ float3 operator*(float3 a, float b)
{
	return make_float3(a.x * b, a.y * b, a.z * b);
}

__host__ __device__ void operator+=(float3& a, float3 b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
}



//This is run on the GPU, like an opengl shader except their is no "pipeline" only 
// code that runs per core (32 of these kernels per warp).
//
// The way each thread identifies itself is a little strange; the index value here
// is computed as (block_index * block_size + thread_index). As a reference, I have
// set the BlockDim for this kernal to 256, so threadIdx will go 0-255. The block
// dimension is an arbitary size of your choice (suggest power of 2), that corresponds
// to the amount of threads to run simultanously together. If your kernels require alot
// of memory (shared mem or local mem) or have set the L2 cache to be large, then you will
// be able to fit less threads into each block. If performance is critical, which if your
// using CUDA for your own projects, it probably is, then I suggest profiling the block
// dimension alot and attempting to reduce register usage as much as possible (see CUDA
// profiler for more details on kernal runtime)
__global__ void kernel_UpdateParticles(uint num_particles, float dt, float3* vertex_buf, float3* vel_arr) {
	///Note: This line will often look like an error because the defines are only valid with the cuda compiler
	///      not the c++ compiler. Just trust it /does/ compile fine :)
	uint index = blockIdx.x*blockDim.x + threadIdx.x; 
	if (index >= num_particles)
		return;

	//Read in memory for global memory
	float3 pos = vertex_buf[index];
	float3 vel = vel_arr[index];

	//Add gravity to particles
	vel.y -= 9.81f * dt;
	pos += vel * dt;


	//If pos goes below zero, lets just reset him at the centre of our 'emitter'
	if (pos.y < 0.0f)
	{
		pos = make_float3(0.f, 0.f, 0.f);

		initialize_y_vel(vel);
	}

	//Write our updated particle back to global memory
	vertex_buf[index] = pos;
	vel_arr[index] = vel;
}










//All the code below this point is ONLY executed on the CPU

CudaParticles::CudaParticles()
	: num_particles(0)
	, cGLPositions(NULL)
	, cArrVelocities(NULL)
{

}

CudaParticles::~CudaParticles()
{
	if (cArrVelocities)
	{
		gpuErrchk(cudaFree(cArrVelocities));
		cArrVelocities = NULL;
	}

	if (cGLPositions)
	{
		gpuErrchk(cudaGraphicsUnregisterResource(cGLPositions));
		cGLPositions = NULL;
	}
}


//For this example, we will be showing how easy it is to cross CUDA and opengl is,
// so instead of creating 2 cuda arrays, we will create a cuda array for velocities
// and use the pre-built opengl vertex buffer passed in (this could be from an nclgl
// mesh class etc)
void CudaParticles::InitializeArrays(uint size, GLuint glVertexBuffer)
{
	num_particles = size;

//Velocity Array - CUDA Array
	//First we need some 'random' velocities for our particles
	float3* tmp_vels = new float3[size];
	for (uint i = 0; i < size; ++i)
	{
		float angle = (float)DegToRad((rand() % 18000) / 50.f);
		float power = (rand() % 1000) / 500.f - 1.f;
		tmp_vels[i] = make_float3(
			cos(angle) * power,
			0.f,
			sin(angle)* power
		);
		


		initialize_y_vel(tmp_vels[i]);
	}

	//Allocate our GPU memory
	gpuErrchk(cudaMalloc(&cArrVelocities, size * sizeof(float3)));

	//Copy our local memory over
	// - This is a generic copy function and as such we need to tell it if we
	//   are copying from the CPU to GPU or back again.
	gpuErrchk(cudaMemcpy(cArrVelocities, tmp_vels, size * sizeof(float3), cudaMemcpyHostToDevice));


	//Cleanup tmp velocities as they now happily live on the GPU
	delete[] tmp_vels;


//Position Array - OpenGL Buffer Resource
	//The map flags here can be none (read/write), writeDiscard (write only) or readOnly.
	gpuErrchk(cudaGraphicsGLRegisterBuffer(&cGLPositions, glVertexBuffer, cudaGraphicsMapFlagsNone));
}


//We need some code to launch our kernel, this is executed on the CPU (__host__)
// and will split up our workload into 'blocks' and fire off all of the
// work for the GPU to split up and process using the above kernel code.
void CudaParticles::UpdateParticles(uint num_verts_to_update, float dt)
{
	num_verts_to_update = min(num_verts_to_update, num_particles);

	//First thing we need to do is 'lock' the opengl vertex buffer down, this
	// will lock it's memory in place and stop the driver from moving it's location
	// ..and also provide us with a memory pointer which we can use inside our kernel
	// to change read/write it's contents. :)
	size_t tmpVertexPtrSize;
	float3 *tmpVertexPtr;
	gpuErrchk(cudaGraphicsMapResources(1, &cGLPositions, 0));
	gpuErrchk(cudaGraphicsResourceGetMappedPointer((void **)&tmpVertexPtr, &tmpVertexPtrSize, cGLPositions));

	if (tmpVertexPtrSize < num_particles * sizeof(float3))
	{
		NCLERROR("OpenGL vertex buffer not large enough to encompass all our particles!");
		return;
	}


	// This is where we specify the block dimension, and number of blocks to fire
	// - See kernel_UpdateParticles comment for more information.
	// The y/z components here are always 1 as we only want to split along the x axis
	// for a single index value. For very large arrays, or 2D/3D textures etc you may
	// want index_x and index_y values aswell.
	dim3 block(256, 1, 1);
	/// We just want to do ceil(num_verts_to_update/block.x) here, but to avoid possible floating point
	/// errors, we can accomplish the same 'ceil' operation in integer math with: 
	/// [int->float->int]ceil(num_verts_to_update/block.x) = [integer only](num_verts_to_update + block.x - 1) / block.x
	dim3 grid((num_verts_to_update + block.x - 1) / block.x, 1, 1);

	//This is the code that actually 'runs' our kernel
	///This (again) will likely look like an intelisense error, but trust
	/// me, it /does/ compile alright.
	kernel_UpdateParticles<<< grid, block >>>(num_verts_to_update, dt, tmpVertexPtr, cArrVelocities);



	// Finally we need to unmap our OpenGL vertex buffer, allowing the driver
	// control over where it resides again.
	gpuErrchk(cudaGraphicsUnmapResources(1, &cGLPositions, 0));
}
