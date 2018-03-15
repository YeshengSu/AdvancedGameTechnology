#version 150 core

in vec3 position;
out vec4 OutFrag;

void main(void)	{
	OutFrag = vec4(position, 1.0f);
}