/******************************************************************************
Class: ScreenPicker
Implements:
Author:
Pieran Marris <p.marris@newcastle.ac.uk>
Description:
Uses picking as a means to allow mouse interactivity with world space objects.
Any renderable object can be clicked/hovered over etc and only requires registering/unregistering
itself to the Screen Picker instance when it wants to start/stop listening for mouse events.

This class works by rendering all objects in the scene to an off-screen texture, with
all clickable objects outputing their array index to the framebuffer. This is then sampled at
the mouse position and the relevent objects mouse functions are called. In order to get the
world space position of the mouse at a given location the depth buffer is also sampled and
multiplied by the inverse (projMtx*viewMtx) similar to how deferred rendering converts screen
space coordinates back into world space.

!!WARNING!!!
If you've reached this comment because you are stumbling through the headers trying desperately
to find out why the engine is so slow and the "Scene Updates" are apparently taking 3ms+ for a
simple scene and 0ms when clicking an object... then yes.. you have reached the source of the problem. =[

The method of reading pixels back from the graphics card is 'very' slow, as it requires the entire
rendering pipeline to stall while pixels are read back to the cpu. There are ways this can be optimized
by doing the reads asynchronously through the use of pixel buffers, however this wont aliaviate the big
issue - a whole extra render pass. As you progress through the physics module and hopefully implement a
broadphase you will find doing a ray cast through the broadphase a much faster means of doing
mouse-interactivity than screen picking and the way all professional game engines will handle it.

In the extra tutorials for Graphics for Games there is a tutorial on RayCasting for picking purposes,
which can be extended (with the new physics collision shapes) to provide an accurate picking solution.

*//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <nclgl\TSingleton.h>
#include <nclgl\RenderNode.h>
#include <nclgl\Shader.h>
#include <GL\glew.h>

//Our texture only stores 16bit unsigned shorts, so has a hard limit on the number of values it can store. 
//  Hopefully you will never be able to trigger this value though. 
#define MAX_PICKABLE_OBJECTS 65534


//Callback function called each frame the user clicks (and holds) the given object
//Params:
//	dt				- Elapsed time (in seconds) since last frame
//  new_pos			- The new world space position of the dragged object
//  pos_change		- The amount (in world space) the object has been moved since the last frame
//  isStopClicking	- Set to true if the object is about to be released (OnMouseUp) and should stop any dragging behaviour
typedef std::function<void(float dt, const Vector3& new_pos, const Vector3& pos_change, bool isStopClicking)> OnMouseDownCallback;


//In the screen picker we need to store a couple of extra bits of information about the render nodes.
typedef struct
{
	RenderNode*			_renderNode;
	OnMouseDownCallback _callback;
} PickerNode;


//Use async texture reads (ping-pong pixel buffer objects)
//  Reads the current mouse pixel into a pixel buffer asynchronously while using the pixel buffer from 
//	the previous frame to compute all mouse interaction.
//
//  If enabled, this will result in all picker information being one frame behind the actual result. This
//  probably isn't great for FPS style environment, but it much quicker and doesn't stall the graphics
//  pipeline to copy data.
#define USE_ASYNC_PBO


//Nsight (at the moment) doesn't support glReadPixels(format: GL_RED_INTEGER) so to debug everything 
// we can just treat all indices as float's and do lossy conversion back into unsigned shorts. This
// is not ideal, but it's better than nothing
#define USE_NSIGHT_HACK


typedef unsigned short ushort;


class ScreenPicker : public TSingleton<ScreenPicker>
{
	friend class TSingleton<ScreenPicker>;
	friend class GraphicsPipeline;

public:

	//Add object to list of 'clickable' objects to be tested 
	// - Optional callback which if set will be called each frame while the RenderNode is pressed/held
	void RegisterNodeForMouseCallback(RenderNode* node, OnMouseDownCallback callback = NULL);

	//Remove object from the list of 'clickable' objects
	void UnregisterNodeForMouseCallback(RenderNode* node);




protected:
	//Called by ScreenRenderer
	void ClearAllObjects();
	void RenderPickingScene(const Matrix4& projViewMtx,
		const Matrix4& invProjViewMtx,
		GLuint depthTex,
		uint depthTexWidth,
		uint depthTexHeight);

	//Internal FBO/Shader generation
	void UpdateAssets(int screen_width, int screen_height);

	//ScreenRenderer Update Phase
	//  - Returns true if an object has been clicked
	bool HandleMouseClicks(float dt);

	//Internal handling of different mouse state scenarios
	void HandleObjectMouseHover(PickerNode* target);
	void HandleObjectMouseLeave();
	void HandleObjectMouseDown(PickerNode* target);
	void HandleObjectMouseUp(float dt, Vector3& clip_space);
	void HandleObjectMouseMove(float dt, Vector3& clip_space);

	//Pseodo Protected

	ScreenPicker();
	virtual ~ScreenPicker();

protected:
	void SamplePickerFBO(const Vector2& mouse_pos, ushort& out_idx, float& out_depth);

protected:

	//Array of all objects to be tested
	std::vector<PickerNode> m_AllRegisteredObjects;

	//Current State
	Vector4			m_CurrentObjectBaseColor;
	PickerNode*		m_pCurrentlyHoverObject;
	PickerNode*		m_pCurrentlyHeldObject;

	//Cached data to allow world-space movement computation
	Vector3			m_ObjOffset;
	float			m_OldDepth;
	Vector3			m_OldWorldSpacePos;

	//clip-space to world-space transform
	Matrix4			m_invViewProjMtx;

	//Shader
	Shader* m_pShaderPicker;

	//Framebuffer
	int		m_TexWidth;
	int		m_TexHeight;
	GLuint	m_glPickerFBO;
	GLuint	m_glPickerTex;

	//Frame Pick'd information
	ushort	m_PickerIdx;
	float	m_PickerDepth;

#ifdef USE_ASYNC_PBO
	uint	m_pboIdx; //1 or 0
	GLuint	m_glPBO[2];
#endif
};