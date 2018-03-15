#version 330 core

#define SHADOWMAP_NUM  4

//Per object
uniform sampler2D  	uDiffuseTex;
uniform vec4		uColor;

//Constant Per Frame
uniform mat3  		uNormalMtx;
uniform float 		uRadius;
uniform vec3  		uCameraPos;
uniform vec3		uAmbientColor;
uniform vec3		uLightDirection;
uniform float		uSpecularFactor;

//Shadow
uniform sampler2DArrayShadow uShadowTex;
uniform vec2  		uShadowSinglePixel;
uniform mat4 		uShadowTransform[SHADOWMAP_NUM];





in Vertex	{
	smooth vec3 	 worldPos;
	smooth vec2 	 rPos;
	flat float       depth_min;
	flat float       depth_max;
} IN;



const float NORMAL_BIAS = 0.002f;
const float RAW_BIAS 	= 0.00025f;

out vec4 OutFrag;


float DoShadowTest(vec3 tsShadow, int tsLayer, vec2 pix)
{	
	vec4 tCoord;
	tCoord.xyw = tsShadow;	//I don't know why...
	tCoord.z = tsLayer;

	//Far
	if (tsLayer > 0)
	{
		/* Far away shadowmaps
		Fast single shadow sample 
		*/
		return texture(uShadowTex, tCoord);
	}
	else
	{
		/* Near shadowmaps (or in case of looking in the light dir, the centre of the screen)
		PCF filtering
		   - Takes a 4x4 sample around each pixel and averages the result, bluring the edges of shadows
		   - Requires 16 linearly interpolated samples which can be quite expensive
		   - Consider looking into exponential shadow mapping as a nicer looking and faster way to achieve soft shadowing.
		 */
		float shadow = 0.0f;
		for (float y = -1.5f; y <= 1.5f; y += 1.0f)
			for (float x = -1.5f; x <= 1.5f; x += 1.0f)
				shadow += texture(uShadowTex, tCoord + vec4(pix.x * x, pix.y * y, 0, 0));
			
		return shadow / 16.0f;
	}
}




void main(void)	{

	float distSq = dot(IN.rPos, IN.rPos);
	if (distSq > 1.f)
	{
		discard;
	}
	float z = sqrt(1.0f - distSq);
	
	float ndc_depth = mix(IN.depth_max, IN.depth_min, z);	
	float depth = (((gl_DepthRange.far-gl_DepthRange.near) * ndc_depth) + gl_DepthRange.near + gl_DepthRange.far) / 2.0;
	gl_FragDepth = depth;
	
	OutFrag = uColor;
	
	
	
	vec3 normal = uNormalMtx * vec3(IN.rPos.x, IN.rPos.y, z);

	vec3 worldPos = IN.worldPos;
	worldPos += uNormalMtx[2] * (z * uRadius);
	
//Shadow Calculations
	vec4 shadowWsPos = vec4(worldPos.xyz + normal * NORMAL_BIAS, 1.0f);
	
	float shadow = 1.0f;
	for (int layerIdx = 0; layerIdx < SHADOWMAP_NUM; layerIdx++)
	{
		//We don't need to divide by w for orthographic projections
		vec4 hcsShadow = uShadowTransform[layerIdx] * shadowWsPos;
	
		if (abs(hcsShadow.x) <= 1.0f && abs(hcsShadow.y) <= 1.0f)
		{
			hcsShadow.z -= RAW_BIAS;
			hcsShadow.xyz = hcsShadow.xyz * 0.5f + 0.5f;

			shadow = DoShadowTest(hcsShadow.xyz, layerIdx, uShadowSinglePixel);
			break;
		}
	}
	
//Lighting Calculations
	vec3 viewDir 		= normalize(uCameraPos - worldPos.xyz);
	vec3 halfDir 		= normalize(viewDir - uLightDirection);
	float rFactor       = max(0.0, dot(halfDir , normal ));
	
	float dFactor       = max(0.0, dot(-uLightDirection , normal)) ;
    float sFactor       = pow(rFactor , uSpecularFactor );
	   
//Color Computations
	const float specIntensity = 0.4f;
	
	//Quick hack to approximate specular colour of an object, assuming the light colour is white
	vec3 specColor = min(uColor.rgb + vec3(0.5f), vec3(1)); 
	
//Output Final Colour
	vec3 diffuse  = uColor.rgb * dFactor * shadow;
	vec3 specular = specColor * sFactor * shadow * specIntensity;
	
	//Combine lighting
	OutFrag.xyz 	= diffuse + specular + uColor.rgb* uAmbientColor;
	OutFrag.a 		= uColor.a;	
}