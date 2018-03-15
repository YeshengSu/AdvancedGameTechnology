#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4  uProjMtx;
uniform mat4  uViewMtx;

in Vertex	{
	flat vec4 color;
	flat vec4 pos;
} IN[];

out Vertex	{
	flat   vec4 color;
	smooth vec2 rPos;
	flat   float depth_min;
	flat   float depth_max;
} OUT;

void main()  
{  
	OUT.color 	 = IN[0].color;
	
	vec4 p0 	 = uViewMtx * gl_in[0].gl_Position;
	float radius = IN[0].pos.w;

	vec4 near_depth = uProjMtx * (p0 + vec4(0,0,radius, 0));
	vec4 far_depth = uProjMtx * p0;
	OUT.depth_min = near_depth.z / near_depth.w;
	OUT.depth_max = far_depth.z / far_depth.w;
	

	OUT.rPos = vec2(-1, 1);
	gl_Position = uProjMtx * (p0 + vec4(-radius, radius, 0, 0));	
	EmitVertex();	
	
	OUT.rPos = vec2(-1, -1);
	gl_Position = uProjMtx * (p0 + vec4(-radius, -radius, 0, 0));	
	EmitVertex();
	
	OUT.rPos = vec2(1, 1);
	gl_Position = uProjMtx * (p0 + vec4(radius, radius, 0, 0));	
	EmitVertex();
	
	OUT.rPos = vec2(1, -1);
	gl_Position = uProjMtx * (p0 + vec4(radius, -radius, 0, 0));	
	EmitVertex();	
}  