#version 330 core

uniform uint uPickerIdx;

out float OutFrag;

void main(void)
{
	OutFrag = float(uPickerIdx);
}