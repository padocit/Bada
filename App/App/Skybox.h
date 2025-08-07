#pragma once

#include "Typedef.h"
#include "IRenderer.h"
#include "LinkedList.h"

class CGame;

class CSkybox
{
public:
	CSkybox();
	~CSkybox();

	BOOL	Initialize(CGame* pGame);
	void	SetPosition(float x, float y, float z);
	void	SetScale(float x, float y, float z);
	void	SetRotationY(float fRotY);
	void	Run();
	void	Render();

public:
	SORT_LINK	m_LinkInGame;

private:
	IMeshObject* CreateSkyboxMeshObject();

	void	UpdateTransform();
	void	Cleanup();

private:
	CGame* m_pGame = nullptr;
	IRenderer* m_pRenderer = nullptr;
	IMeshObject* m_pMeshObj = nullptr;

	XMVECTOR m_Scale = { 1.0f, 1.0f, 1.0f, 0.0f };
	XMVECTOR m_Pos = {};
	float m_fRotY = 0.0f;

	XMMATRIX m_matScale = {};
	XMMATRIX m_matRot = {};
	XMMATRIX m_matTrans = {};
	XMMATRIX m_matWorld = {};
	BOOL	m_bUpdateTransform = FALSE;

};