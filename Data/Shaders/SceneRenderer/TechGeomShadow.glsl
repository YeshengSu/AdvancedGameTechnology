#version 330 core
#define SHADOWMAP_NUM  4
#define SHADOWMAP_NUM_VERTS  12

layout(triangles) in;
layout(triangle_strip, max_vertices = SHADOWMAP_NUM_VERTS) out; 

uniform mat4 uShadowTransform[4];

out vec3 position;
void main()  
{  	
	for (int layer = 0; layer < SHADOWMAP_NUM; ++layer)
	{
		for (int i = 0; i < gl_in.length(); ++i) {
			gl_Position = uShadowTransform[layer] * gl_in[i].gl_Position;
			gl_Layer = layer;
			position = gl_Position.xyz;
			EmitVertex();
		}
		EndPrimitive();
	}
}  