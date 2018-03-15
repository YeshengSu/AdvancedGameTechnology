#version 330 core

uniform uint uPickerIdx;

out uint OutFrag;

void main(void)
{
	OutFrag = uPickerIdx;
}