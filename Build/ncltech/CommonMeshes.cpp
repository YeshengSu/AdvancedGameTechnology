#include "CommonMeshes.h"
#include <nclgl\NCLDebug.h>
#include <nclgl\OBJMesh.h>
#include <SOIL.h>

Mesh* CommonMeshes::m_pCube		= NULL;
Mesh* CommonMeshes::m_pSphere	= NULL;

GLuint    CommonMeshes::m_pCheckerboardTex = 0;

void CommonMeshes::InitializeMeshes()
{
	if (m_pCube == NULL)
	{
		m_pCheckerboardTex = SOIL_load_OGL_texture(TEXTUREDIR"checkerboard.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_COMPRESS_TO_DXT);

		if (m_pCheckerboardTex)
		{
			glBindTexture(GL_TEXTURE_2D, m_pCheckerboardTex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //No linear interpolation to get crisp checkerboard no matter the scalling
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			NCLERROR("Unable to load checkerboard texture!");
		}

		m_pCube = new OBJMesh(MESHDIR"cube.obj");
		m_pCube->SetTexture(m_pCheckerboardTex);

		m_pSphere = new OBJMesh(MESHDIR"sphere.obj");
		m_pSphere->SetTexture(m_pCheckerboardTex);
	}
}

void CommonMeshes::ReleaseMeshes()
{
	SAFE_DELETE(m_pCube);
	SAFE_DELETE(m_pSphere);
	if (m_pCheckerboardTex)
	{
		glDeleteTextures(1, &m_pCheckerboardTex);
		m_pCheckerboardTex = 0;
	}
}