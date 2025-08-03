#include "Pch.h"
#include "IRenderer.h"
#include "GameObject.h"
#include "Game.h"
#include "Global.h"

CGame::CGame()
{

}
CGame::~CGame()
{
	Cleanup();
}

BOOL CGame::Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV)
{
	CreateRendererInstance(&m_pRenderer);
	m_pRenderer->Initialize(hWnd, bEnableDebugLayer, bEnableGBV);
	m_hWnd = hWnd;

	// Create Font
	m_pFontObj = m_pRenderer->CreateFontObject(L"Tahoma", 18.0f);

	// Create text-texture
	m_TextImageWidth = 512;
	m_TextImageHeight = 256;
	m_pTextImage = (BYTE*)malloc(m_TextImageWidth * m_TextImageHeight * 4);
	m_pTextTexHandle = m_pRenderer->CreateDynamicTexture(m_TextImageWidth, m_TextImageHeight);
	memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);

	m_pSpriteObjCommon = m_pRenderer->CreateSpriteObject();

	// Create GameObjects
	const DWORD GAME_OBJ_COUNT = 2000;
	for (DWORD i = 0; i < GAME_OBJ_COUNT; i++)
	{
		CGameObject* pGameObj = CreateGameObject();
		if (pGameObj)
		{
			float x = (float)((rand() % 21) - 10);	// [-10m, 10m] 
			float y = 0.0f;
			float z = (float)((rand() % 21) - 10);	// [-10m, 10m]
			pGameObj->SetPosition(x, y, z);
			float rad = (rand() % 181) * (3.1415f / 180.0f);
			pGameObj->SetRotationY(rad);
		}
	}
	m_pRenderer->SetCameraPos(0.0f, 0.0f, -10.0f);
	return TRUE;
}

CGameObject* CGame::CreateGameObject()
{
	CGameObject* pGameObj = new CGameObject;
	pGameObj->Initialize(this);
	LinkToLinkedListFIFO(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);

	return pGameObj;
}

void CGame::OnKeyDown(UINT nChar, UINT uiScanCode)
{
	switch (nChar)
	{
	case VK_SHIFT:
		m_bShiftKeyDown = TRUE; // TODO:
		break;
	case 'W':
		m_CamOffsetZ = 0.05f;
		break;
	case 'S':
		m_CamOffsetZ = -0.05f;
		break;
	case 'A':
		m_CamOffsetX = -0.05f;
		break;
	case 'D':
		m_CamOffsetX = 0.05f;
		break;
	case 'E':
		m_CamOffsetY = 0.05f;
		break;
	case 'Q':
		m_CamOffsetY = -0.05f;
		break;
	}
}

void CGame::OnKeyUp(UINT nChar, UINT uiScanCode)
{
	switch (nChar)
	{
	case VK_SHIFT:
		m_bShiftKeyDown = FALSE; // TODO:
		break;
	case 'W':
		m_CamOffsetZ = 0.0f;
		break;
	case 'S':
		m_CamOffsetZ = 0.0f;
		break;
	case 'A':
		m_CamOffsetX = 0.0f;
		break;
	case 'D':
		m_CamOffsetX = 0.0f;
		break;
	case 'E':
		m_CamOffsetY = 0.0f;
		break;
	case 'Q':
		m_CamOffsetY = 0.0f;
		break;
	}
}

void CGame::Run()
{
	m_FrameCount++;

	// Begin
	ULONGLONG CurTick = GetTickCount64();

	// Game business logic
	Update(CurTick);

	Render();

	if (CurTick - m_PrvFrameCheckTick > 1000)
	{
		m_PrvFrameCheckTick = CurTick;

		WCHAR wchTxt[64];
		m_FPS = m_FrameCount;
		m_dwCommandListCount = m_pRenderer->GetCommandListCount();

		swprintf_s(wchTxt, L"FPS: %u, CommandList: %u", m_FPS, m_dwCommandListCount);
		SetWindowText(m_hWnd, wchTxt);

		m_FrameCount = 0;
	}
}
BOOL CGame::Update(ULONGLONG CurTick)
{
	// Update Scene with 60FPS (less than real-framecount)
	if (CurTick - m_PrvUpdateTick < 16)
	{
		return FALSE;
	}
	m_PrvUpdateTick = CurTick;

	// Update camra
	if (m_CamOffsetX != 0.0f || m_CamOffsetY != 0.0f || m_CamOffsetZ != 0.0f)
	{
		m_pRenderer->MoveCamera(m_CamOffsetX, m_CamOffsetY, m_CamOffsetZ);
	}

	// Update game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Run();
		pCur = pCur->pNext;
	}

	// Update status text
	int iTextWidth = 0;
	int iTextHeight = 0;
	WCHAR	wchTxt[64] = {};
	DWORD	dwTxtLen = swprintf_s(wchTxt, L"FrameRate: %u, CommandList: %u", m_FPS, m_dwCommandListCount);

	if (wcscmp(m_wchText, wchTxt))
	{
		// Text is updated.
		memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);
		m_pRenderer->WriteTextToBitmap(m_pTextImage, m_TextImageWidth, m_TextImageHeight, m_TextImageWidth * 4, &iTextWidth, &iTextHeight, m_pFontObj, wchTxt, dwTxtLen);
		m_pRenderer->UpdateTextureWithImage(m_pTextTexHandle, m_pTextImage, m_TextImageWidth, m_TextImageHeight);
		wcscpy_s(m_wchText, wchTxt);
	}
	else
	{
		int a = 0;
	}
	return TRUE;
}

void CGame::Render()
{
	m_pRenderer->BeginRender();

	// Render game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	DWORD dwObjCount = 0;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Render();
		pCur = pCur->pNext;
		dwObjCount++;
	}

	// Render dynamic texture as text
	m_pRenderer->RenderSpriteWithTex(m_pSpriteObjCommon, 512 + 5, 256 + 5 + 256 + 5, 1.0f, 1.0f, nullptr, 0.0f, m_pTextTexHandle);

	// End
	m_pRenderer->EndRender();

	// Present
	m_pRenderer->Present();
}

BOOL CGame::UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight)
{
	BOOL bResult = FALSE;
	if (m_pRenderer)
	{
		bResult = m_pRenderer->UpdateWindowSize(dwBackBufferWidth, dwBackBufferHeight);
	}
	return bResult;
}

void CGame::DeleteGameObject(CGameObject* pGameObj)
{
	UnLinkFromLinkedList(&m_pGameObjLinkHead, &m_pGameObjLinkTail, &pGameObj->m_LinkInGame);
	delete pGameObj;
}
void CGame::DeleteAllGameObjects()
{
	while (m_pGameObjLinkHead)
	{
		CGameObject* pGameObj = (CGameObject*)m_pGameObjLinkHead->pItem;
		DeleteGameObject(pGameObj);
	}
}

void CGame::Cleanup()
{
	DeleteAllGameObjects();

	if (m_pTextImage)
	{
		free(m_pTextImage);
		m_pTextImage = nullptr;
	}
	if (m_pRenderer)
	{
		if (m_pFontObj)
		{
			m_pRenderer->DeleteFontObject(m_pFontObj);
			m_pFontObj = nullptr;
		}

		if (m_pTextTexHandle)
		{
			m_pRenderer->DeleteTexture(m_pTextTexHandle);
			m_pTextTexHandle = nullptr;
		}
		if (m_pSpriteObjCommon)
		{
			m_pSpriteObjCommon->Release();
			m_pSpriteObjCommon = nullptr;
		}

		m_pRenderer->Release();
		m_pRenderer = nullptr;
	}
}