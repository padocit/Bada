#pragma once

#include "Typedef.h"
#include "IRenderer.h"
#include "LinkedList.h"
#include "GameObject.h"

class CSkybox;
class CGame
{
public:
	CGame();
	~CGame();

	BOOL	Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV);
	void	Run();
	BOOL	Update(ULONGLONG CurTick);
	void	OnKeyDown(UINT nChar, UINT uiScanCode);
	void	OnKeyUp(UINT nChar, UINT uiScanCode);
	void	OnMouse(WORD mouseX, WORD mouseY);
	void	OnMouseMove(WORD mouseX, WORD mouseY);
	void	OnMouseLeftDown(WORD mouseX, WORD mouseY);
	void	OnMouseLeftUp(WORD mouseX, WORD mouseY);
	void	OnMouseRightDown(WORD mouseX, WORD mouseY);
	void	OnMouseRightUp(WORD mouseX, WORD mouseY);
	BOOL	UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);

	IRenderer* INL_GetRenderer() const { return m_pRenderer; }

private:
	void	Render();
	CGameObject* CreateGameObject(PRIMITIVE_TYPE primitiveType = PRIMITIVE_TYPE_BOX);
	void	DeleteGameObject(CGameObject* pGameObj);
	void	DeleteAllGameObjects();

	void	Cleanup();

private:
	HMODULE m_hRendererDLL = nullptr;
	IRenderer*	m_pRenderer = nullptr;
	HWND	m_hWnd = nullptr;
	ISprite* m_pSpriteObjCommon = nullptr;

	BYTE* m_pTextImage = nullptr;
	UINT m_TextImageWidth = 0;
	UINT m_TextImageHeight = 0;
	void* m_pTextTexTexHandle = nullptr;
	void* m_pFontObj = nullptr;

	BOOL	m_bShiftKeyDown = FALSE;
	BOOL	m_bLeftMouseDown = FALSE;
	BOOL	m_bRightMouseDown = FALSE;

	float m_CamOffsetX = 0.0f;
	float m_CamOffsetY = 0.0f;
	float m_CamOffsetZ = 0.0f;
	float m_CamMoveSpeed = 5.0f;

	int m_MouseX = 0;
	int m_MouseY = 0;
	int m_PrevMouseX = 0;
	int m_PrevMouseY = 0;
	int deltaX = 0;
	int deltaY = 0;

	float m_fMouseSensitivity = 0.005f;

	// Game Objects
	CSkybox* m_pSkybox = nullptr;
	SORT_LINK*	m_pGameObjLinkHead = nullptr;
	SORT_LINK*	m_pGameObjLinkTail = nullptr;

	
	ULONGLONG m_PrvFrameCheckTick = 0;
	ULONGLONG m_PrvUpdateTick = 0;
	DWORD	m_FrameCount = 0;
	DWORD	m_FPS = 0;
	DWORD	m_dwCommandListCount = 0;
	WCHAR m_wchText[64] = {};

	WCHAR	m_wchAppPath[_MAX_PATH] = {};
};