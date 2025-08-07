#include "Pch.h"
#include <Windows.h>
#include <DirectXMath.h>
#include "Typedef.h"
#include "IRenderer.h"
#include "LinkedList.h"
#include "VertexUtil.h"
#include "Game.h"
#include "Skybox.h"

CSkybox::CSkybox()
{
	m_LinkInGame.pItem = this;
	m_LinkInGame.pNext = nullptr;
	m_LinkInGame.pPrv = nullptr;

	m_matScale = XMMatrixIdentity();
	m_matRot = XMMatrixIdentity();
	m_matTrans = XMMatrixIdentity();
	m_matWorld = XMMatrixIdentity();
}
CSkybox::~CSkybox()
{
	Cleanup();
}

BOOL CSkybox::Initialize(CGame* pGame)
{
	BOOL bResult = FALSE;
	CGame* m_pGame = pGame;
	m_pRenderer = pGame->INL_GetRenderer();

	m_pMeshObj = CreateSkyboxMeshObject();
	if (m_pMeshObj)
	{
		SetScale(100.0f, 100.0f, 100.0f);	// Skybox 크기 설정
		bResult = TRUE;
	}
	return bResult;

}
void CSkybox::UpdateTransform()
{
	// world matrix = scale x rotation x trasnlation
	m_matWorld = XMMatrixMultiply(m_matScale, m_matRot);
	m_matWorld = XMMatrixMultiply(m_matWorld, m_matTrans);
}
void CSkybox::SetPosition(float x, float y, float z)
{
	m_Pos.m128_f32[0] = x;
	m_Pos.m128_f32[1] = y;
	m_Pos.m128_f32[2] = z;

	m_matTrans = XMMatrixTranslation(x, y, z);

	m_bUpdateTransform = TRUE;
}
void CSkybox::SetScale(float x, float y, float z)
{
	m_Scale.m128_f32[0] = x;
	m_Scale.m128_f32[1] = y;
	m_Scale.m128_f32[2] = z;

	m_matScale = XMMatrixScaling(x, y, z);

	m_bUpdateTransform = TRUE;
}
void CSkybox::SetRotationY(float fRotY)
{
	m_fRotY = fRotY;
	m_matRot = XMMatrixRotationY(fRotY);

	m_bUpdateTransform = TRUE;
}

void CSkybox::Run()
{
	// per 30FPS or 60 FPS
	if (m_bUpdateTransform)
	{
		UpdateTransform();
		m_bUpdateTransform = FALSE;
	}
	else
	{
		int a = 0;
	}
}
void CSkybox::Render()
{
	if (m_pMeshObj)
	{
		m_pRenderer->RenderSkybox(m_pMeshObj, &m_matWorld);
	}
}

IMeshObject* CSkybox::CreateSkyboxMeshObject()
{
	IMeshObject* pMeshObj = nullptr;

	// create box mesh
	// create vertices and indices
	WORD	pIndexList[36] = {};
	BasicVertex* pVertexList = nullptr;
	DWORD dwVertexCount = CreateBoxMesh(&pVertexList, pIndexList, (DWORD)_countof(pIndexList), 1.0f, TRUE);

	// create SkyboxMeshObject from Renderer
	pMeshObj = m_pRenderer->CreateSkyboxMeshObject();

	const WCHAR* wchTexFileName = L"../../Assets/DGarden_specularIBL.dds";

	// Set meshes to the SkyboxMeshObject
	pMeshObj->BeginCreateMesh(pVertexList, dwVertexCount, 6);
	for (DWORD i = 0; i < 6; i++)
	{
		pMeshObj->InsertTriGroup(pIndexList + i * 6, 2, wchTexFileName); // CubeMap texture
	}
	pMeshObj->EndCreateMesh();

	// delete vertices and indices
	if (pVertexList)
	{
		DeleteBoxMesh(pVertexList);
		pVertexList = nullptr;
	}
	return pMeshObj;
}
void CSkybox::Cleanup()
{
	if (m_pMeshObj)
	{
		m_pMeshObj->Release();
		m_pMeshObj = nullptr;
	}
}
