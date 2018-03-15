#version 330 core

uniform sampler2D uFontTex;

in Vertex	{
	flat   vec4 color;
	smooth vec2 texCoords;
} IN;

out vec4 OutFrag;

void main(void)	{
	float font = texture(uFontTex, vec2(IN.texCoords.x, 1.f - IN.texCoords.y)).r;
	if (font < 0.1f)
	{
		discard;
	}
	
	OutFrag = IN.color * font;
	

}