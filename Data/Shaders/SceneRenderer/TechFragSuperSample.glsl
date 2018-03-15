#version 330 core

uniform sampler2D uColorTex;

uniform vec2 	uSinglepixel;
uniform float	uGammaCorrection;
uniform float	uNumSuperSamples;

	
in Vertex	{
	vec2 texCoord;
} IN;

out vec4 OutFrag;

void main(void)	{
	//We need to inverse the gamma function to go back to a non-linear color ramp
	float invGammaCorrection = 1.0 / uGammaCorrection;

	//Sample every other pixel and interpolate
	vec3 color = vec3(0.0f);
	float samples_taken = 0.0f;
	for (float x = 0.0f; x < uNumSuperSamples; x += 2.0f)
	{
		for (float y = 0.0f; y < uNumSuperSamples; y += 2.0f)
		{
			color += texture(uColorTex, IN.texCoord + uSinglepixel * vec2(x, y)).xyz;
			samples_taken += 1.0f;
		}
	}
	color = color / samples_taken;
	
	
	//Inverse Gamma Ramp
	color = pow(color, vec3(invGammaCorrection));
	OutFrag = vec4(color, 1.0f);
}