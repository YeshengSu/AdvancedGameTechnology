#version 330 core

layout (location = 0) in  vec4 position;
layout (location = 1) in  vec4 color;

out Vertex {
	flat vec4 color;
	flat vec4 pos;	
} OUT;

void main(void)	{
	gl_Position	  = vec4(position.xyz, 1.0f);
	OUT.pos		  = position;	
	OUT.color     = color;
}