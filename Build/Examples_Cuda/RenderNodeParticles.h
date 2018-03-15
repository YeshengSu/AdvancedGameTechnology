#pragma once

//Due to the way we handle drawing via nclgl, there isn't
// an easy way to add a shader to a single object.. but
// here's is a hacky way of getting around it.
//
// This isn't about the graphics though, so just don't read
// too much into this class and it's horrible-ness ;)
//
// - At it's core, it's an opengl vertex buffer which exists
//   as an array of Vector3's. On a draw call, it will draw
//   these to the screen as a 3D sphere object centred on 
//   the vertices position.


#include <nclgl\RenderNode.h>

class RenderNodeParticles : public RenderNode
{
public:
	RenderNodeParticles()
		: RenderNode() 
		, shaderShadowParticles(NULL)
		, shaderLightParticles(NULL)
		, numVertices(0)
		, glVertexArray(NULL)
		, glVertexBuffer(NULL)
		, particleRadius(0.1f)

	{
		glGenVertexArrays(1, &glVertexArray);

		shaderShadowParticles = new Shader(
			SHADERDIR"Particles/ParticleShadowVertex.glsl", 
			SHADERDIR"Particles/ParticleShadowFragment.glsl",			
			SHADERDIR"Particles/ParticleShadowGeometry.glsl");

		if (!shaderShadowParticles->LinkProgram())
		{
			NCLERROR("Unable to load particle shadow shader!");
		}

		shaderLightParticles = new Shader(
			SHADERDIR"Particles/ParticleLightVertex.glsl",
			SHADERDIR"Particles/ParticleLightFragment.glsl",
			SHADERDIR"Particles/ParticleLightGeometry.glsl");

		if (!shaderLightParticles->LinkProgram())
		{
			NCLERROR("Unable to load particle light shader!");
		}
		
	}

	virtual ~RenderNodeParticles()
	{
		SAFE_DELETE(shaderLightParticles);
		SAFE_DELETE(shaderShadowParticles);

		if (glVertexArray)
		{
			glDeleteVertexArrays(1, &glVertexArray);
			glVertexArray = 0;
		}

		if (glVertexBuffer)
		{
			glDeleteBuffers(1, &glVertexBuffer);
			glVertexBuffer = 0;
		}
	}

	virtual bool IsRenderable() override
	{
		return true;
	}

	GLuint GetGLVertexBuffer()
	{
		return glVertexBuffer;
	}

	void SetParticleRadius(float worldspace_size)
	{
		particleRadius = worldspace_size;
	}

	//Generate new vertex buffer
	// if initial_values is not null, they will be uploaded to the gfx driver as the
	// the default positions to render.
	void GeneratePositionBuffer(uint size, Vector3* initial_values = NULL)
	{
		numVertices = size;

		glBindVertexArray(glVertexArray);
		if (!glVertexBuffer)
		{
			glGenBuffers(1, &glVertexBuffer);
		}

		glBindBuffer(GL_ARRAY_BUFFER, glVertexBuffer);
		if (initial_values == NULL)
		{
			//The position buffer is read from cuda, so needs to be initialized to [0,0,0]'s
			Vector3* tmp = new Vector3[size];
			memset(tmp, 0, size * sizeof(Vector3));
			glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), tmp, GL_STATIC_DRAW);
			delete[] tmp;
		}
		else
		{
			glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vector3), initial_values, GL_STATIC_DRAW);
		}
		glVertexAttribPointer(VERTEX_BUFFER, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(VERTEX_BUFFER);

		glBindVertexArray(0);
	}

	virtual void DrawOpenGL(bool isShadowPass) override
	{
		if (numVertices == 0)
			return;


		//Hack: We need to use our custom shader just for this object,
		//      but nclgl/graphicspipeline doesn't sort/handle by shader,
		//      so we just need to draw as normal but reset the shader
		//      back to graphics default after render.
		GLint oldShader;
		glGetIntegerv(GL_CURRENT_PROGRAM, &oldShader);

		if (isShadowPass)
		{
			SetupShadowShader();
		}
		else
		{
			SetupLightShader();
		}
		
		glDisable(GL_CULL_FACE);
		glBindVertexArray(glVertexArray);
		glDrawArrays(GL_POINTS, 0, numVertices);
		glBindVertexArray(0);
		glEnable(GL_CULL_FACE);
		//Reset shader back to original
		glUseProgram(oldShader);
	}


	void SetupShadowShader()
	{
		const Matrix4* projMtxs = GraphicsPipeline::Instance()->GetShadowProjMatrices();
		const Matrix4& viewMtx = GraphicsPipeline::Instance()->GetShadowViewMtx();

		glUseProgram(shaderShadowParticles->GetProgram());
		glUniformMatrix4fv(glGetUniformLocation(shaderShadowParticles->GetProgram(), "uShadowProj[0]"), SHADOWMAP_NUM, GL_FALSE, (float*)&projMtxs[0]);
		glUniformMatrix4fv(glGetUniformLocation(shaderShadowParticles->GetProgram(), "uShadowView"), 1, GL_FALSE, viewMtx.values);
		glUniform1f(glGetUniformLocation(shaderShadowParticles->GetProgram(), "uRadius"), particleRadius);
	}

	void SetupLightShader()
	{
		const Matrix4& projMtx = GraphicsPipeline::Instance()->GetProjMtx();
		const Matrix4& viewMtx = GraphicsPipeline::Instance()->GetViewMtx();
		const Matrix3 normalMtx = Matrix3::Transpose(Matrix3(viewMtx));

		glUseProgram(shaderLightParticles->GetProgram());
		glUniformMatrix4fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uViewMtx"), 1, GL_FALSE, (float*)&viewMtx);
		glUniformMatrix4fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uProjMtx"), 1, GL_FALSE, (float*)&projMtx);
		glUniformMatrix3fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uNormalMtx"), 1, GL_FALSE, (float*)&normalMtx);

		glUniform1f(glGetUniformLocation(shaderLightParticles->GetProgram(), "uRadius"), particleRadius);
		glUniform4fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uColor"), 1, &color.x);

		glUniform3fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uCameraPos"), 1, 
			(float*)&GraphicsPipeline::Instance()->GetCamera()->GetPosition());
		glUniform3fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uAmbientColor"), 1, 
			(float*)&GraphicsPipeline::Instance()->GetAmbientColor());
		glUniform3fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uLightDirection"), 1, 
			(float*)&GraphicsPipeline::Instance()->GetLightDirection());
		glUniform1fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uSpecularFactor"), 1, 
			&GraphicsPipeline::Instance()->GetSpecularFactor());

		glUniformMatrix4fv(glGetUniformLocation(shaderLightParticles->GetProgram(), "uShadowTransform[0]"), SHADOWMAP_NUM, GL_FALSE, 
			(float*)&GraphicsPipeline::Instance()->GetShadowProjViewMatrices()[0]);
		glUniform1i(glGetUniformLocation(shaderLightParticles->GetProgram(), "uShadowTex"), 2);
		glUniform2f(glGetUniformLocation(shaderLightParticles->GetProgram(), "uShadowSinglePixel"), 1.f / SHADOWMAP_SIZE, 1.f / SHADOWMAP_SIZE);

		glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D_ARRAY, GraphicsPipeline::Instance()->GetShadowTex());
	}


protected:

	float particleRadius;

	Shader* shaderLightParticles;
	Shader* shaderShadowParticles;

	uint numVertices;
	GLuint glVertexArray;
	GLuint glVertexBuffer;
};