#pragma once
#include <nclgl\OGLRenderer.h>
#include <nclgl\TSingleton.h>
#include <nclgl\Camera.h>
#include <nclgl\RenderNode.h>

//---------------------------
//------ Base Renderer ------
//---------------------------
//------ Use this as a base for incorporating your own graphics
//
//class GraphicsPipeline : public TSingleton<GraphicsPipeline>, OGLRenderer
//{
//	friend class TSingleton<GraphicsPipeline>;
//
//public:
//	//Set all defaults
//	// - Called each time a scene is swapped in/reset, so should reset camera position etc
//	void InitializeDefaults();
//
//	//Add/Remove Render objects to renderable object lists
//	void AddRenderNode(RenderNode* node);
//	void RemoveRenderNode(RenderNode* node);
//
//
//
//
//	//Called by main game loop
//	// - Naming convention from oglrenderer
//	void UpdateScene(float dt);
//	void RenderScene();
//
//
//
//	//Utils
//	inline Camera* GetCamera();
//
//protected:
//	GraphicsPipeline() : OGLRenderer(Window::GetWindow()) {}
//	virtual ~GraphicsPipeline() {}
//
//protected:
//};




//-------------------
// Default Implementation
//-----------------------
// This is a GameTech module, not graphics. This default renderer
// should be good enough for all physics and AI madness you want 
// to mess around with.



//Number of cascading shadow maps 
// - As we don't have the ability to set shadow defines, any changes here also need to be
//   mirrored inside "TechFragForwardRender.glsl" and "TechGeomShadow.glsl" shaders.
#define SHADOWMAP_NUM 4			

//Size of the shadows maps in pixels
// - With a size of 4096x4096 (and 4 shadowmaps) using 32bit floats this
//   currently results in shadows using up 256MB of space. Which is quite alot,
//   but also currently the only potentially memory sensitive thing we do in this 
//   renderer so it's fine.
#define SHADOWMAP_SIZE 4096


#define PROJ_FAR      50.0f			//Can see for 50m - setting this too far really hurts shadow quality as they attempt to cover the entirety of the view frustum
#define PROJ_NEAR     0.1f			//Nearest object @ 10cm
#define PROJ_FOV      45.0f			//45 degree field of view

typedef std::pair<RenderNode*, float> TransparentPair;


class GraphicsPipeline : public TSingleton<GraphicsPipeline>, OGLRenderer
{
	friend class TSingleton<GraphicsPipeline>;

public:
	//Set all defaults
	// - Called each time a scene is swapped in/reset, so should reset camera position etc
	void InitializeDefaults();

	//Add/Remove Render objects to renderable object lists
	void AddRenderNode(RenderNode* node);
	void RemoveRenderNode(RenderNode* node);




	//Called by main game loop
	// - Naming convention from oglrenderer
	virtual void UpdateScene(float dt) override;
	virtual void RenderScene() override;



	//Utils
	inline Camera* GetCamera() { return camera; }
	inline bool GetVsyncEnabled() const { return isVsyncEnabled; }
	inline void SetVsyncEnabled(bool enabled) { wglSwapIntervalEXT((isVsyncEnabled = enabled) ? 1 : 0); }

protected:
	GraphicsPipeline();
	virtual ~GraphicsPipeline();

	virtual void Resize(int x, int y) override; //Called by window when it is resized

	void LoadShaders();
	void UpdateAssets(int width, int height);
	void BuildAndSortRenderLists();
	void RecursiveAddToRenderLists(RenderNode* node);
	void RenderAllObjects(bool renderTransparentBothSides, std::function<void(RenderNode*)> perObjectFunc = NULL);
	void BuildShadowTransforms(); //Builds the shadow projView matrices

protected:
	Matrix4 projViewMatrix;

	//Render FBO
	GLuint				screenTexWidth, screenTexHeight;
	GLuint				screenFBO;
	GLuint				screenTexColor;
	GLuint				screenTexDepth;

	//Shaders
	Shader* shaderPresentToWindow;
	Shader* shaderShadow;
	Shader* shaderForwardLighting;

	//Render Params
	Vector3	ambientColor;
	float	gammaCorrection;	//Monitor Default: 1.0 / 2.2 (Where 2.2 here is the gamma of the monitor which we need to invert before doing lighting calculations)		
	Vector3	lightDirection;
	Vector3 backgroundColor;
	float	specularFactor;
	uint	numSuperSamples;


	//Shadowmaps
	float	sceneBoundingRadius; ///Approx based on scene contents
	GLuint	shadowFBO;
	GLuint	shadowTex;
	Matrix4	shadowProj[SHADOWMAP_NUM];
	float   normalizedFarPlanes[SHADOWMAP_NUM - 1];

	//Common
	Mesh* fullscreenQuad;
	Camera* camera;
	bool isVsyncEnabled;
	std::vector<RenderNode*> allNodes;

	std::vector<RenderNode*> renderlistOpaque;
	std::vector<TransparentPair> renderlistTransparent;	//Also stores cameraDist in the second argument for sorting purposes
};