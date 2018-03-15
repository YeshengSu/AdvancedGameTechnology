#version 330 core

uniform mat4 uProjViewMtx;

layout (location = 0) in  vec4 position;
layout (location = 1) in  vec4 color;

out Vertex {
	smooth vec4 color;
	smooth vec4 pos;	
} OUT;

void main(void)	{
	vec4 vp = uProjViewMtx * vec4(position.xyz, 1.0f);
	gl_Position	  = vp;
	
	OUT.pos		  = vec4(vp.xyz, position.w);	
	OUT.color    = color;
}