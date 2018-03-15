#version 330 core

layout (location = 0) in  vec4 position;
layout (location = 1) in  vec4 color;

out Vertex {
	flat vec4 pos;
	flat vec4 color;
} OUT;

void main(void)	{
	gl_Position	  = position;
	OUT.pos		  = position;	
	OUT.color     = color;
}