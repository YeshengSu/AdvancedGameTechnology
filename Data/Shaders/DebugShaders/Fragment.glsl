#version 330 core

in Vertex	{
	flat vec4 color;
} IN;

out vec4 OutFrag;

void main(void)	{
	OutFrag = IN.color;
}