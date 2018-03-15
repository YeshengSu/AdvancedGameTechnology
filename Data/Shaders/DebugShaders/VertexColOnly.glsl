//This is exactly the same as Vertex.glsl, however without outputting position.
// - This seemed to be causing linking issues on some the newer drivers

#version 330 core

uniform mat4 uProjViewMtx;

layout (location = 0) in  vec4 position;
layout (location = 1) in  vec4 color;

out Vertex {
	flat vec4 color;
} OUT;

void main(void)	{
	gl_Position  = uProjViewMtx * vec4(position.xyz, 1.0f);
	
	OUT.color    = color;
}