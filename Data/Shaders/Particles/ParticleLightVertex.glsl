#version 330 core

layout (location = 0) in  vec3 position;

out Vertex {
	flat vec3 pos;	
} OUT;

void main(void)	{
	gl_Position	  = vec4(position, 1.0f);
	OUT.pos		  = position;	
}