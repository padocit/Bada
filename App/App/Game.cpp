#include "pch.h"
#include <Windows.h>
#include <DirectXMath.h>
#include "Typedef.h"
#include "IRenderer.h"
#include "LinkedList.h"
#include "Game.h"
#include "Skybox.h"
#include <shlwapi.h>


CGame::CGame()
{

}
BOOL CGame::Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV)
{
	const WCHAR* wchRendererFileName = nullptr;

#if defined(_M_ARM64EC) || defined(_M_ARM64)
	#ifdef _DEBUG
		wchRendererFileName = L"../../Bin/windows-x86_64/Debug/App/BadaDX12.dll";
	#else
		wchRendererFileName = L"../../Bin/windows-x86_64/Release/App/BadaDX12.dll";
#endif
#elif defined(_M_AMD64)
	#ifdef _DEBUG
		wchRendererFileName = L"../../Bin/windows-x86_64/Debug/App/BadaDX12.dll";
	#else
		wchRendererFileName = L"../../Bin/windows-x86_64/Release/App/BadaDX12.dll";
	#endif
#elif defined(_M_IX86)
	#ifdef _DEBUG
		wchRendererFileName = L"../../Bin/windows-x86/Debug/App/BadaDX12.dll";
	#else
		wchRendererFileName = L"../../Bin/windows-x86/Release/App/BadaDX12.dll";
	#endif
#endif
	WCHAR	wchErrTxt[128] = {};
	DWORD	dwErrCode = 0;

	m_hRendererDLL = LoadLibrary(wchRendererFileName);
	if (!m_hRendererDLL)
	{
		dwErrCode = GetLastError();
		swprintf_s(wchErrTxt, L"Fail to LoadLibrary(%s) - Error Code: %u", wchRendererFileName, dwErrCode);
		MessageBox(hWnd, wchErrTxt, L"Error", MB_OK);
		__debugbreak();
	}
	CREATE_INSTANCE_FUNC	pCreateFunc = (CREATE_INSTANCE_FUNC)GetProcAddress(m_hRendererDLL, "DllCreateInstance");
	pCreateFunc(&m_pRenderer);

	// Get App Path and Set Shader Path
	WCHAR wchShaderPath[_MAX_PATH] = {};
	HMODULE hModule = GetModuleHandle(nullptr);
	if (hModule)
	{
		WCHAR wchOldPath[_MAX_PATH] = {};
		GetCurrentDirectory(_MAX_PATH, wchOldPath);

		int ret = GetModuleFileName(hModule, m_wchAppPath, _MAX_PATH);
		PathRemoveFileSpec(m_wchAppPath);
		SetCurrentDirectory(m_wchAppPath);
		SetCurrentDirectory(L"../../../../Bada/BadaDX12/Shaders");
		GetCurrentDirectory(_MAX_PATH, wchShaderPath);

		SetCurrentDirectory(wchOldPath);
	}
	m_pRenderer->Initialize(hWnd, bEnableDebugLayer, bEnableGBV, wchShaderPath);
	m_hWnd = hWnd;

	// Create Font
	m_pFontObj = m_pRenderer->CreateFontObject(L"Tahoma", 18.0f);
	
	// create texture for draw text
	m_TextImageWidth = 512;
	m_TextImageHeight = 256;
	m_pTextImage = (BYTE*)malloc(m_TextImageWidth * m_TextImageHeight * 4);
	m_pTextTexTexHandle = m_pRenderer->CreateDynamicTexture(m_TextImageWidth, m_TextImageHeight);
	memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);

	m_pSpriteObjCommon = m_pRenderer->CreateSpriteObject();

	// create skybox
	m_pSkybox = new CSkybox;
	m_pSkybox->Initialize(this);

	CGameObject* pGameObjBox = CreateGameObject(PRIMITIVE_TYPE_BOX);
	if (pGameObjBox)
	{
		float x = -1.0f;
		float y = 0.0f;
		float z = 1.5f;
		pGameObjBox->SetPosition(x, y, z);
		//float rad = 45.0f;
		//pGameObjBox->SetRotationY(rad);
	}

	CGameObject* pGameObjSphere = CreateGameObject(PRIMITIVE_TYPE_SPHERE);
	if (pGameObjSphere)
	{
		float x = 1.0f;
		float y = 0.0f;
		float z = 1.5f;
		pGameObjSphere->SetPosition(x, y, z);
		float rad = 0.0f;
		pGameObjSphere->SetRotationY(rad);
	}

	m_pRenderer->SetCameraPos(0.0f, 0.0f, 0.0f);

	return TRUE;
}

CGameObject* CGame::CreateGameObject(PRIMITIVE_TYPE primitiveType)
{
	// meshobject를 공용으로 쓰도록 한다.
	//__debugbreak();
	CGameObject* pGameObj = new CGameObject;
	pGameObj->Initialize(this, primitiveType);
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
		m_CamOffsetZ = 1.0f;
		break;
	case 'S':
		m_CamOffsetZ = -1.0f;
		break;
	case 'A':
		m_CamOffsetX = -1.0f;
		break;
	case 'D':
		m_CamOffsetX = 1.0f;
		break;
	case 'E':
		m_CamOffsetY = 1.0f;
		break;
	case 'Q':
		m_CamOffsetY = -1.0f;
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

//TODO:
// Game-Renderer-Camera의 관계

void CGame::OnMouse(WORD mouseX, WORD mouseY)
{
	m_MouseX = (int)mouseX;
	m_MouseY = (int)mouseY;
}

void CGame::OnMouseMove(WORD mouseX, WORD mouseY)
{
	OnMouse(mouseX, mouseY);

    // 이전 위치와 비교하여 델타 계산
    if (m_bRightMouseDown)
    {
		int deltaX = m_MouseX - m_PrevMouseX;
		int deltaY = m_MouseY - m_PrevMouseY;

		// 카메라 회전
		if (deltaX != 0 || deltaY != 0)
		{
			float rotationX = (float)deltaX * m_fMouseSensitivity;  // Yaw
			float rotationY = (float)deltaY * m_fMouseSensitivity;  // Pitch

			m_pRenderer->MouseRotateCameraDelta(rotationX, rotationY);
		}
	}
	m_PrevMouseX = m_MouseX;
	m_PrevMouseY = m_MouseY;
}

void CGame::OnMouseLeftDown(WORD mouseX, WORD mouseY)
{
	OnMouse(mouseX, mouseY);
	if (!m_bLeftMouseDown)
	{
		m_bLeftMouseDown = TRUE;
	}
}
void CGame::OnMouseLeftUp(WORD mouseX, WORD mouseY)
{
	if (m_bLeftMouseDown)
	{
		m_bLeftMouseDown = FALSE;
	}
}
void CGame::OnMouseRightDown(WORD mouseX, WORD mouseY)
{
    OnMouse(mouseX, mouseY);
    if (!m_bRightMouseDown)
    {
        m_bRightMouseDown = TRUE;

		// 첫 번째 위치 설정
		m_PrevMouseX = m_MouseX;
		m_PrevMouseY = m_MouseY;
    }
}
void CGame::OnMouseRightUp(WORD mouseX, WORD mouseY)
{
	if (m_bRightMouseDown)
	{
		m_bRightMouseDown = FALSE;
	}
}

void CGame::Run()
{
	m_FrameCount++;

	// begin
	ULONGLONG CurTick = GetTickCount64();

	// game business logic
	Update(CurTick);

	Render();

	if (CurTick - m_PrvFrameCheckTick > 1000)
	{
		m_PrvFrameCheckTick = CurTick;	
				
		WCHAR wchTxt[64];
		m_FPS = m_FrameCount;
		m_dwCommandListCount = m_pRenderer->GetCommandListCount();

		swprintf_s(wchTxt, L"FPS : %u, CommandList : %u ", m_FPS, m_dwCommandListCount);
		SetWindowText(m_hWnd, wchTxt);
				
		m_FrameCount = 0;
	}
}
BOOL CGame::Update(ULONGLONG CurTick)
{
	// Update Scene with about 144FPS(6.94ms)
	if (CurTick - m_PrvUpdateTick < 7)
	{
		return FALSE;
	}

	float deltaTime = (float)(CurTick - m_PrvUpdateTick) / 1000.0f;
	m_PrvUpdateTick = CurTick;

	// Update camera
	if (m_bRightMouseDown)
	{	
		// Translate
		if (m_CamOffsetX != 0.0f || m_CamOffsetY != 0.0f || m_CamOffsetZ != 0.0f)
		{
			float moveX = m_CamOffsetX * m_CamMoveSpeed * deltaTime;
			float moveY = m_CamOffsetY * m_CamMoveSpeed * deltaTime;
			float moveZ = m_CamOffsetZ * m_CamMoveSpeed * deltaTime;

			m_pRenderer->MoveCamera(moveX, moveY, moveZ);
		}
	}

	// update skybox
	if (m_pSkybox)
	{
		m_pSkybox->Run();
	}

	// update game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Run();
		pCur = pCur->pNext;
	}
	
	// update status text
	int iTextWidth = 0;
	int iTextHeight = 0;
	WCHAR	wchTxt[64] = {};
	DWORD	dwTxtLen = swprintf_s(wchTxt, L"FrameRate: %u, CommandList: %u", m_FPS, m_dwCommandListCount);

	if (wcscmp(m_wchText, wchTxt))
	{
		// 텍스트가 변경된 경우
		memset(m_pTextImage, 0, m_TextImageWidth * m_TextImageHeight * 4);
		m_pRenderer->WriteTextToBitmap(m_pTextImage, m_TextImageWidth, m_TextImageHeight, m_TextImageWidth * 4, &iTextWidth, &iTextHeight, m_pFontObj, wchTxt, dwTxtLen);
		m_pRenderer->UpdateTextureWithImage(m_pTextTexTexHandle, m_pTextImage, m_TextImageWidth, m_TextImageHeight);
		wcscpy_s(m_wchText, wchTxt);
	}
	else
	{
		// 텍스트가 변경되지 않은 경우 - 업데이트 할 필요 없다.
		int a = 0;
	}
	return TRUE;
}
void CGame::Render()
{
	m_pRenderer->BeginRender();

	// render skybox
	if (m_pSkybox)
	{
		m_pSkybox->Render();
	}

	// render game objects
	SORT_LINK* pCur = m_pGameObjLinkHead;
	DWORD dwObjCount = 0;
	while (pCur)
	{
		CGameObject* pGameObj = (CGameObject*)pCur->pItem;
		pGameObj->Render();
		pCur = pCur->pNext;
		dwObjCount++;
	}	
	// render dynamic texture as text
	//m_pRenderer->RenderSpriteWithTex(m_pSpriteObjCommon, 0, 0, 1.0f, 1.0f, nullptr, 0.0f, m_pTextTexTexHandle);

	// end
	m_pRenderer->EndRender();

	// Present
	m_pRenderer->Present();
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
BOOL CGame::UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight)
{
	BOOL bResult = FALSE;
	if (m_pRenderer)
	{
		bResult = m_pRenderer->UpdateWindowSize(dwBackBufferWidth, dwBackBufferHeight);
	}
	return bResult;
}
void CGame::Cleanup()
{
	DeleteAllGameObjects();

	if (m_pSkybox)
	{
		delete m_pSkybox;
		m_pSkybox = nullptr;
	}

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
	
		if (m_pTextTexTexHandle)
		{
			m_pRenderer->DeleteTexture(m_pTextTexTexHandle);
			m_pTextTexTexHandle = nullptr;
		}
		if (m_pSpriteObjCommon)
		{
			m_pSpriteObjCommon->Release();
			m_pSpriteObjCommon = nullptr;
		}

		m_pRenderer->Release();
		m_pRenderer = nullptr;
	}
	if (m_hRendererDLL)
	{
		FreeLibrary(m_hRendererDLL);
		m_hRendererDLL = nullptr;
	}
}
CGame::~CGame()
{
	Cleanup();
}


