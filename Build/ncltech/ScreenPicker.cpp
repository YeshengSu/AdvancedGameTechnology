#include "ScreenPicker.h"
#include "GraphicsPipeline.h"
#include <nclgl\NCLDebug.h>

ScreenPicker::ScreenPicker()
	: m_pCurrentlyHeldObject(NULL)
	, m_pCurrentlyHoverObject(NULL)
	, m_TexWidth(0)
	, m_TexHeight(0)
	, m_glPickerFBO(NULL)
	, m_glPickerTex(NULL)
	, m_pShaderPicker(NULL)
	, m_PickerIdx(0)
	, m_PickerDepth(0.0f)
{
#ifdef USE_ASYNC_PBO
	memset(m_glPBO, 0, 2 * sizeof(GLuint));
#endif
}

ScreenPicker::~ScreenPicker()
{
	m_pCurrentlyHeldObject = NULL;
	m_pCurrentlyHoverObject = NULL;
	m_AllRegisteredObjects.clear();

	SAFE_DELETE(m_pShaderPicker);

	if (m_glPickerTex)
	{
		glDeleteTextures(1, &m_glPickerTex);
		glDeleteFramebuffers(1, &m_glPickerFBO);
		m_glPickerTex = NULL;
	}

#ifdef USE_ASYNC_PBO
	if (m_glPBO[0])
	{
		glDeleteBuffers(2, m_glPBO);
		m_glPBO[0] = NULL;
	}
#endif
}

void ScreenPicker::RegisterNodeForMouseCallback(RenderNode* node, OnMouseDownCallback callback)
{

	if (!node)
	{
		NCLERROR("ATTEMPTING TO REGISTER NULL FOR SCREENPICKER CALLBACK");
		return;
	}

	//Make sure the node doesn't already exist
	for (PickerNode& pnode : m_AllRegisteredObjects)
	{
		if (pnode._renderNode == node)
		{
			NCLERROR("ATTEMPTING TO REGISTER NODE FOR PICKING MULTIPLE TIMES");
			return;
		}
	}

	if (m_AllRegisteredObjects.size() >= MAX_PICKABLE_OBJECTS)
	{
		NCLERROR("MAX SCREEN PICKER ITEM COUNT REACHED!");
		return;
	}


	PickerNode pnode;
	pnode._renderNode = node;
	pnode._callback = callback;

	m_AllRegisteredObjects.push_back(pnode);
}

void ScreenPicker::UnregisterNodeForMouseCallback(RenderNode* node)
{
	if (!node)
	{
		return;
	}

	uint i = 0;
	for (; i < m_AllRegisteredObjects.size(); ++i)
	{
		if (m_AllRegisteredObjects[i]._renderNode == node)
		{
			auto idx = m_AllRegisteredObjects.begin() + i;
			m_AllRegisteredObjects.erase(idx, idx + 1);
			break;
		}
	}
}

void ScreenPicker::UpdateAssets(int screen_width, int screen_height)
{
	//Have to load the shader here incase the screenpicker constructor is called before we have an OGL context
	if (!m_pShaderPicker)
	{
#ifdef USE_NSIGHT_HACK
		m_pShaderPicker = new Shader(SHADERDIR"SceneRenderer/TechVertexPicker.glsl", SHADERDIR"SceneRenderer/TechFragScreenPicker_nsightfix.glsl");
#else
		m_pShaderPicker = new Shader(SHADERDIR"SceneRenderer/TechVertexPicker.glsl", SHADERDIR"SceneRenderer/TechFragScreenPicker.glsl");
#endif
		glBindFragDataLocation(m_pShaderPicker->GetProgram(), 0, "OutFrag");
		if (!m_pShaderPicker->LinkProgram())
			NCLERROR("Unable to build ScreenPicker Shader!");

	}

	if (m_TexWidth != screen_width || m_TexHeight != screen_height)
	{
		//Build Textures
		m_TexWidth = screen_width;
		m_TexHeight = screen_height;

		if (!m_glPickerTex) glGenTextures(1, &m_glPickerTex);
		glBindTexture(GL_TEXTURE_2D, m_glPickerTex);
#ifdef USE_NSIGHT_HACK
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, m_TexWidth, m_TexHeight, 0, GL_RED, GL_FLOAT, NULL);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, m_TexWidth, m_TexHeight, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
#endif
		//Build FBO
		if (!m_glPickerFBO) glGenFramebuffers(1, &m_glPickerFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, m_glPickerFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_glPickerTex, 0);

		//Validate our framebuffer
		GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{

			NCLERROR("Unable to create ScreenPicker Framebuffer! StatusCode: %x", status);
		}
	}

#ifdef USE_ASYNC_PBO
	
	if (!m_glPBO[0])
	{
		glGenBuffers(2, m_glPBO);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPBO[0]);
		glBufferData(GL_PIXEL_PACK_BUFFER, 2 * sizeof(float), 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPBO[1]);
		glBufferData(GL_PIXEL_PACK_BUFFER, 2 * sizeof(float), 0, GL_STREAM_READ);  //We allocate 2 extra bytes here to make sure 4 byte alignment can be satisfied

		m_pboIdx = 0;
	}
#endif
}

void ScreenPicker::SamplePickerFBO(const Vector2& mousepos, ushort& out_idx, float& out_depth)
{
#ifdef USE_NSIGHT_HACK
#define READ_PICKER_IDX(void_ptr) glReadPixels((int)(mousepos.x * scalar), (int)(mousepos.y * scalar), 1, 1, GL_RED, GL_FLOAT, void_ptr)
#define READ_PICKER_PBO(ptr) (ushort)(*reinterpret_cast<float*>(ptr + sizeof(float)))
#else
#define READ_PICKER_IDX(void_ptr) glReadPixels((int)(mousepos.x * scalar), (int)(mousepos.y * scalar), 1, 1, GL_RED_INTEGER, GL_UNSIGNED_SHORT, void_ptr)
#define READ_PICKER_PBO(ptr) *reinterpret_cast<unsigned short*>(ptr + sizeof(float))
#endif

	float scalar = m_TexWidth / Window::GetWindow().GetScreenSize().x;

	glBindFramebuffer(GL_FRAMEBUFFER, m_glPickerFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

#ifdef USE_ASYNC_PBO
	//Start reading from FBO to spare buffer (ready for next frame)
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPBO[m_pboIdx]);
	glReadPixels((int)(mousepos.x * scalar), (int)(mousepos.y * scalar), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	READ_PICKER_IDX( (void*)(sizeof(float)) );

	m_pboIdx = (m_pboIdx + 1) & 0x1;

	//Read values from current buffer (buffered from previous frame)
	glBindBuffer(GL_PIXEL_PACK_BUFFER, m_glPBO[m_pboIdx]);
	char* ptr = reinterpret_cast<char*>(glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY));
	if (ptr)
	{
		out_depth = *(reinterpret_cast<float*>(ptr));
		out_idx = READ_PICKER_PBO(ptr);
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
	}
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
#else
	glReadPixels((int)(mousepos.x * scalar), (int)(mousepos.y * scalar), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &out_depth);
#ifdef USE_NSIGHT_HACK
	float f;
	READ_PICKER_IDX(&f);
	out_idx = (ushort)f;
#else
	READ_PICKER_IDX(&out_idx);
#endif
#endif
}

bool ScreenPicker::HandleMouseClicks(float dt)
{
	if (m_AllRegisteredObjects.size() > 0 && m_TexWidth > 0 && m_TexHeight > 0)
	{

		Vector2 mousepos;
		bool mouseInWindow = Window::GetWindow().GetMouseScreenPos(&mousepos);
		mousepos.y = Window::GetWindow().GetScreenSize().y - mousepos.y; //Flip Y as opengl uses bottom left as origin

		Vector3 clipspacepos = Vector3(
			mousepos.x / (Window::GetWindow().GetScreenSize().x * 0.5f) - 1.0f,
			mousepos.y / (Window::GetWindow().GetScreenSize().y * 0.5f) - 1.0f,
			0.0f);


		bool mouseDown = Window::GetMouse()->ButtonDown(MOUSE_LEFT) || Window::GetMouse()->ButtonDown(MOUSE_RIGHT);
		bool mouseHeld = Window::GetMouse()->ButtonHeld(MOUSE_LEFT) || Window::GetMouse()->ButtonHeld(MOUSE_RIGHT);

		//Do we have an object already being dragged?
		if (m_pCurrentlyHeldObject != NULL)
		{
			if (Window::GetMouse()->GetWheelMovement() != 0)
			{
				const float projD = PROJ_FAR - PROJ_NEAR;
				const float projFN = PROJ_FAR + PROJ_NEAR;
				const float projWP = 2.0f * PROJ_NEAR * PROJ_FAR;
				const float proj10 = projFN / projD;
				const float proj14 = projWP / projD;

				float depth_d = (m_OldDepth * 2.0f - 1.0f) * projD;
				float oldDepthWS = projWP / (projFN - depth_d);

				oldDepthWS += Window::GetMouse()->GetWheelMovement();

				float newDepthClip = -((proj14 - oldDepthWS * proj10) / oldDepthWS);
				if (fabs(newDepthClip) <= 1.f)
					m_OldDepth = newDepthClip * 0.5f + 0.5f;
			}

			//Is the object still being dragged?
			if (!mouseDown || !mouseInWindow)
			{
				HandleObjectMouseUp(dt, clipspacepos);
			}
			else
			{
				HandleObjectMouseMove(dt, clipspacepos);
			}
			return true;
		}
		else if (m_glPickerFBO != NULL)
		{
			if (m_PickerIdx > 0 && m_PickerIdx <= m_AllRegisteredObjects.size())
			{
				clipspacepos.z = m_PickerDepth * 2.0f - 1.0f;
				m_OldWorldSpacePos = m_invViewProjMtx * clipspacepos;
				m_OldDepth = m_PickerDepth;

				PickerNode* target_obj = &m_AllRegisteredObjects[m_PickerIdx - 1];

				//Are we clicking the object or just hovering?
				if (!mouseHeld)
				{
					if (mouseDown)
					{
						HandleObjectMouseDown(target_obj);
						return true;
					}
					else
					{
						HandleObjectMouseHover(target_obj);
					}
				}
			}
			else if (m_pCurrentlyHoverObject)
			{
				HandleObjectMouseLeave();
			}
		}
	}
	return false;
}


void ScreenPicker::HandleObjectMouseHover(PickerNode* target)
{
	if (target != m_pCurrentlyHoverObject)
	{
		if (m_pCurrentlyHoverObject)
			HandleObjectMouseLeave();

		m_pCurrentlyHoverObject = target;
		m_CurrentObjectBaseColor = target->_renderNode->GetColor();

		//Set color to highlight color
		target->_renderNode->SetColor(m_CurrentObjectBaseColor + Vector4(0.1f, 0.1f, 0.1f, 0.0f));

		Window::GetWindow().SetCursorStyle(CURSOR_STYLE_GRAB);
	}
}

void ScreenPicker::HandleObjectMouseLeave()
{
	if (m_pCurrentlyHoverObject)
	{
		//Set color back to original object color
		m_pCurrentlyHoverObject->_renderNode->SetColor(m_CurrentObjectBaseColor);
		m_pCurrentlyHoverObject = NULL;

		Window::GetWindow().SetCursorStyle(CURSOR_STYLE_DEFAULT);
	}
}

void ScreenPicker::HandleObjectMouseDown(PickerNode* target)
{
	if (target != m_pCurrentlyHeldObject)
	{
		if (m_pCurrentlyHeldObject)
			NCLERROR("New object clicked while already dragging another object!");

		m_pCurrentlyHeldObject = target;
		m_ObjOffset = target->_renderNode->GetWorldTransform().GetPositionVector() - m_OldWorldSpacePos;

		//Set color to clicked color
		target->_renderNode->SetColor(m_CurrentObjectBaseColor + Vector4(0.2f, 0.2f, 0.2f, 0.0f));
	}
}

void ScreenPicker::HandleObjectMouseUp(float dt, Vector3& clip_space)
{
	if (m_pCurrentlyHeldObject)
	{
		//Set one final 'mouse move' update 
		if (m_pCurrentlyHeldObject->_callback)
		{
			//Compute World Space position
			clip_space.z = m_OldDepth * 2.0f - 1.0f;
			Vector3 newWorldSpacePos = m_invViewProjMtx * clip_space;

			Vector3 worldMovement = newWorldSpacePos - m_OldWorldSpacePos;
			m_OldWorldSpacePos = newWorldSpacePos;
			m_pCurrentlyHeldObject->_callback(dt, newWorldSpacePos + m_ObjOffset, worldMovement, true);
		}

		//Set color back to 'hover' color
		m_pCurrentlyHeldObject->_renderNode->SetColor(m_CurrentObjectBaseColor + Vector4(0.1f, 0.1f, 0.1f, 0.0f));

		m_pCurrentlyHeldObject = NULL;
	}
}

void ScreenPicker::HandleObjectMouseMove(float dt, Vector3& clip_space)
{
	//Check the current object has a mouse move callback function
	if (m_pCurrentlyHeldObject && m_pCurrentlyHeldObject->_callback)
	{
		//Compute World Space position
		clip_space.z = m_OldDepth * 2.0f - 1.0f;
		Vector3 newWorldSpacePos = m_invViewProjMtx * clip_space;

		Vector3 worldMovement = newWorldSpacePos - m_OldWorldSpacePos;
		m_OldWorldSpacePos = newWorldSpacePos;

		m_pCurrentlyHeldObject->_callback(dt, newWorldSpacePos + m_ObjOffset, worldMovement, false);
	}
}


void ScreenPicker::RenderPickingScene(
	const Matrix4& projViewMtx, 
	const Matrix4& invProjViewMtx, 
	GLuint depthTex, 
	uint depthTexWidth, 
	uint depthTexHeight)
{
	m_invViewProjMtx = invProjViewMtx;
	UpdateAssets(depthTexWidth, depthTexHeight);


	//Check to see if we even need an updated picking texture?
	Vector2 mousepos;
	if (m_pCurrentlyHeldObject != NULL || !Window::GetWindow().GetMouseScreenPos(&mousepos))
	{
		return;
	}
	mousepos.y = Window::GetWindow().GetScreenSize().y - mousepos.y;

	//Setup Shader
	glUseProgram(m_pShaderPicker->GetProgram());
	//m_pShaderPicker->SetUniform("uProjViewMtx", &projViewMtx);
	glUniformMatrix4fv(glGetUniformLocation(m_pShaderPicker->GetProgram(), "uProjViewMtx"), 1, GL_FALSE, (float*)&projViewMtx);


	//Bind FBO
	glBindFramebuffer(GL_FRAMEBUFFER, m_glPickerFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex, 0); //Bind the main render pass's depth
	glViewport(0, 0, m_TexWidth, m_TexHeight);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT);



	glDepthFunc(GL_LEQUAL); //So objects we render again (in our list of 'clickables') will still render on top of themselves (with additional offset help)
	glDepthMask(GL_FALSE);  //So we don't change the depth buffer - for instance if we have an 'invisible' clickable object
	glDisable(GL_BLEND);   //To render a single value and not use it for blending

	//As we are drawing these obejcts on top of themselves (in the depth buffer)
	// we need to offset them 1 unit closer in the depth buffer to avoid z-fighting
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.f, -1.f);

	//Render our clickable objects
	GLint uModelMtx  = glGetUniformLocation(m_pShaderPicker->GetProgram(), "uModelMtx");
	GLint uPickerIdx = glGetUniformLocation(m_pShaderPicker->GetProgram(), "uPickerIdx");
	for (uint i = 0; i < m_AllRegisteredObjects.size(); ++i)
	{
		PickerNode& node = m_AllRegisteredObjects[i];
		if (node._renderNode->GetMesh())
		{
			glUniformMatrix4fv(uModelMtx, 1, GL_FALSE, (float*)&node._renderNode->GetWorldTransform());
			glUniform1ui(uPickerIdx, i + 1);

			node._renderNode->GetMesh()->Draw();
		}
	}
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_BLEND);
	glDepthMask(GL_TRUE);


	SamplePickerFBO(mousepos, m_PickerIdx, m_PickerDepth);
}

void ScreenPicker::ClearAllObjects()
{
	m_AllRegisteredObjects.clear();

	if (m_pCurrentlyHoverObject != NULL)
	{
		Window::GetWindow().SetCursorStyle(CURSOR_STYLE_DEFAULT);
	}
	m_pCurrentlyHoverObject = NULL;
	m_pCurrentlyHeldObject = NULL;
}
