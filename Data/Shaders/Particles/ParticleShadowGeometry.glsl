#version 330 core

#define SHADOWMAP_NUM  4
#define SHADOWMAP_NUM_VERTS  16

layout(points) in;
layout(triangle_strip, max_vertices = SHADOWMAP_NUM_VERTS) out;

uniform mat4  uShadowView;
uniform mat4  uShadowProj[4];
uniform float uRadius;

out Vertex	{
	smooth vec2 rPos;
	flat   float depth_min;
	flat   float depth_max;
} OUT;

void main()  
{  
	//Centre of billboard in viewspace
	vec4 p0 	= uShadowView * gl_in[0].gl_Position;
	
	//Corner vertices of quad
	vec4 pA 	= (p0 + vec4(-uRadius, uRadius, 0, 0));
	vec4 pB 	= (p0 + vec4(-uRadius, -uRadius, 0, 0));
	vec4 pC 	= (p0 + vec4(uRadius, uRadius, 0, 0));
	vec4 pD 	= (p0 + vec4(uRadius, -uRadius, 0, 0));

	for (int layerIdx = 0; layerIdx < SHADOWMAP_NUM; ++layerIdx)
	{
		gl_Layer = layerIdx;
			
		vec4 near_depth = uShadowProj[layerIdx] * (p0 + vec4(0,0,uRadius, 0));
		vec4 far_depth = uShadowProj[layerIdx] * p0;
		OUT.depth_min = near_depth.z / near_depth.w;
		OUT.depth_max = far_depth.z / far_depth.w;
		
		OUT.rPos = vec2(-1, 1);
		gl_Position = uShadowProj[layerIdx] * pA;	
		EmitVertex();	
		
		OUT.rPos = vec2(-1, -1);
		gl_Position = uShadowProj[layerIdx] * pB;	
		EmitVertex();
		
		OUT.rPos = vec2(1, 1);
		gl_Position = uShadowProj[layerIdx] * pC;	
		EmitVertex();
		
		OUT.rPos = vec2(1, -1);
		gl_Position = uShadowProj[layerIdx] * pD;	
		EmitVertex();
		
		EndPrimitive();
	}
}  