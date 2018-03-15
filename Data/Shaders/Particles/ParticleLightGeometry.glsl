#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform mat4  uViewMtx;
uniform mat4  uProjMtx;
uniform float uRadius;

in Vertex	{
	flat vec3 pos;
} IN[];

out Vertex	{
	smooth vec3 	 worldPos;
	smooth vec2 	 rPos;
	flat float       depth_min;
	flat float       depth_max;
} OUT;

void OutputVertex(vec2 offset, mat4 projView, vec3 right, vec3 up)
{
	OUT.rPos = offset;
	OUT.worldPos = gl_in[0].gl_Position.xyz 
				+ right * (offset.x * uRadius)
				+ up * (offset.y * uRadius);
	
	gl_Position = projView * vec4(OUT.worldPos, 1.0f);
	EmitVertex();
}

void main()  
{  
	vec3 p0 	 	= (uViewMtx * gl_in[0].gl_Position).xyz;

	vec4 near_depth = uProjMtx * vec4(p0 + vec3(0,0,uRadius), 1.0f);
	vec4 far_depth = uProjMtx * vec4(p0.xyz, 1.0f);
	OUT.depth_min = near_depth.z / near_depth.w;
	OUT.depth_max = far_depth.z / far_depth.w;
	
	
	mat4 projView = uProjMtx * uViewMtx;
	vec3 right = vec3(uViewMtx[0][0], uViewMtx[1][0], uViewMtx[2][0]);
	vec3 up = vec3(uViewMtx[0][1], uViewMtx[1][1], uViewMtx[2][1]);
	
	OutputVertex(vec2(-1, 1), projView, right, up);
	OutputVertex(vec2(-1, -1), projView, right, up);	
	OutputVertex(vec2(1, 1), projView, right, up);
	OutputVertex(vec2(1, -1), projView, right, up);
	
}  