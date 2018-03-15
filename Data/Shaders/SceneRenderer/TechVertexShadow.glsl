#version 330 core

uniform mat4 uModelMtx;

layout (location = 0) in  vec3 position;

void main(void)	
{
	gl_Position	= uModelMtx * vec4(position, 1.0);
}