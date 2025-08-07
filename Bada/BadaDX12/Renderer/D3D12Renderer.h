#pragma once

#include "Typedef.h"
#include "Renderer_typedef.h"
#include "IRenderer.h"

#define USE_MULTI_THREAD

class CD3D12ResourceManager;
class CDescriptorPool;
class CSimpleConstantBufferPool;
class CSingleDescriptorAllocator;
class CConstantBufferManager;
class CFontManager;
class CTextureManager;
class CRenderQueue;
class CCommandListPool;
class CCamera;
struct RENDER_THREAD_DESC;

class CD3D12Renderer : public IRenderer
{
public:
	// Derived from IUnknown
	STDMETHODIMP		 QueryInterface(REFIID, void** ppv);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();

public:
	CD3D12Renderer();
	~CD3D12Renderer();

	// Derived from IRenderer
	BOOL	__stdcall Initialize(HWND hWnd, BOOL bEnableDebugLayer, BOOL bEnableGBV, const WCHAR* wchShaderPath);
	void	__stdcall BeginRender();
	void	__stdcall EndRender();
	void	__stdcall Present();
	BOOL	__stdcall UpdateWindowSize(DWORD dwBackBufferWidth, DWORD dwBackBufferHeight);

	IMeshObject* __stdcall CreateBasicMeshObject();
	IMeshObject* __stdcall CreateSkyboxMeshObject();

	ISprite* __stdcall CreateSpriteObject();
	ISprite* __stdcall CreateSpriteObject(const WCHAR* wchTexFileName, int iPosX, int iPosY, int iWidth, int iHeight);

	void* __stdcall CreateMagentaTexture(UINT texWidth, UINT texHeight);
	void* __stdcall CreateTiledTexture(UINT texWidth, UINT texHeight, DWORD r, DWORD g, DWORD b);
	void* __stdcall CreateDynamicTexture(UINT texWidth, UINT texHeight);
	void* __stdcall CreateTextureFromFile(const WCHAR* wchFileName, BOOL bIsCubeMap = FALSE);
	void  __stdcall DeleteTexture(void* pTexHandle);

	void* __stdcall CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize);
	void  __stdcall DeleteFontObject(void* pFontHandle);
	BOOL  __stdcall WriteTextToBitmap(BYTE* pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int* piOutWidth, int* piOutHeight, void* pFontObjHandle, const WCHAR* wchString, DWORD dwLen);

	void __stdcall RenderMeshObject(IMeshObject* pMeshObj, const XMMATRIX* pMatWorld);
	void __stdcall RenderSkybox(IMeshObject* pMeshObj, const XMMATRIX* pMatWorld);
	void __stdcall RenderSpriteWithTex(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, const RECT* pRect, float Z, void* pTexHandle);
	void __stdcall RenderSprite(void* pSprObjHandle, int iPosX, int iPosY, float fScaleX, float fScaleY, float Z);
	void __stdcall UpdateTextureWithImage(void* pTexHandle, const BYTE* pSrcBits, UINT srcWidth, UINT srcHeight);

	void __stdcall SetCameraPos(float x, float y, float z);
	void __stdcall MoveCamera(float x, float y, float z);
	void __stdcall GetCameraPos(float* pfOutX, float* pfOutY, float* pfOutZ);
	void __stdcall SetCameraRot(float fPitch, float fYaw, float fRoll);
	void __stdcall RotateCamera(float fPitch, float fYaw, float fRoll);
	void __stdcall GetCameraRot(float* pfOutPitch, float* pfOutYaw, float* pfOutRoll);
	void __stdcall MouseRotateCameraDelta(float deltaX, float deltaY);

	//void __stdcall SetCamera(const XMVECTOR* pCamPos, const XMVECTOR* pCamDir, const XMVECTOR* pCamUp);

	DWORD __stdcall GetCommandListCount();

	// for internal
	void EnsureCompleted();
	ID3D12Device5* INL_GetD3DDevice() const { return m_pD3DDevice; }
	CD3D12ResourceManager* INL_GetResourceManager() { return m_pResourceManager; }

	CDescriptorPool* INL_GetDescriptorPool(DWORD dwThreadIndex) { return m_ppDescriptorPool[m_dwCurContextIndex][dwThreadIndex]; }
	CSimpleConstantBufferPool* GetConstantBufferPool(CONSTANT_BUFFER_TYPE type, DWORD dwThreadIndex);

	UINT INL_GetSrvDescriptorSize() const { return m_srvDescriptorSize; }
	CSingleDescriptorAllocator* INL_GetSingleDescriptorAllocator() { return m_pSingleDescriptorAllocator; }
	void GetViewProjMatrix(XMMATRIX* pOutMatView, XMMATRIX* pOutMatProj);
	DWORD INL_GetScreenWidth() const { return m_dwWidth; }
	DWORD INL_GetScreenHeight() const { return m_dwHeight; }
	float INL_GetDPI() const { return m_fDPI; }
	CCamera* INL_GetCamera() const { return m_pCamera; }

	void SetCurrentPathForShader();
	void RestoreCurrentPath();

	// From Render-thread
	void ProcessByThread(DWORD dwThreadIndex);

private:
	static const UINT MAX_DRAW_COUNT_PER_FRAME = 4096;
	static const UINT MAX_DESCRIPTOR_COUNT = 4096;
	static const UINT MAX_RENDER_THREAD_COUNT = 8;

	void InitCamera();

	BOOL CreateHDRRenderTargets(UINT width, UINT height);
	BOOL CreateDepthStencil(UINT width, UINT height);

	void CreateFence();
	void CleanupFence();
	BOOL CreateDescriptorHeapForRTV();
	BOOL CreateDescriptorHeapForHDR();
	BOOL CreateDescriptorHeapForDSV();
	void CleanupDescriptorHeapForRTV();
	void CleanupDescriptorHeapForHDR();
	void CleanupDescriptorHeapForDSV();

	UINT64 Fence();
	void WaitForFenceValue(UINT64 ExpectedFenceValue);

	void Cleanup();

	// For multi-threads
	BOOL InitRenderThreadPool(DWORD dwThreadCount);
	void CleanupRenderThreadPool();

private:
	DWORD m_dwRefCount = 1;
	HWND	m_hWnd = nullptr;
	ID3D12Device5* m_pD3DDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	CD3D12ResourceManager* m_pResourceManager = nullptr;
	CFontManager* m_pFontManager = nullptr;
	CSingleDescriptorAllocator* m_pSingleDescriptorAllocator = nullptr;

	CCommandListPool* m_ppCommandListPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CDescriptorPool* m_ppDescriptorPool[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CConstantBufferManager* m_ppConstBufferManager[MAX_PENDING_FRAME_COUNT][MAX_RENDER_THREAD_COUNT] = {};
	CRenderQueue* m_ppRenderQueue[MAX_RENDER_THREAD_COUNT] = {};
	DWORD m_dwRenderThreadCount = 0;
	DWORD m_dwCurThreadIndex = 0;

	LONG volatile m_lActiveThreadCount = 0;
	HANDLE m_hCompleteEvent = nullptr;
	RENDER_THREAD_DESC* m_pThreadDescList = nullptr;

	CTextureManager* m_pTextureManager = nullptr;

	UINT64 m_pui64LastFenceValue[MAX_PENDING_FRAME_COUNT] = {};
	UINT64 m_ui64FenceValue = 0;


	D3D_FEATURE_LEVEL	m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_ADAPTER_DESC1	m_AdapterDesc = {};
	IDXGISwapChain3* m_pSwapChain = nullptr; // VSync등 기능지원
	D3D12_VIEWPORT  m_Viewport = {};
	D3D12_RECT		m_ScissorRect = {};
	DWORD			m_dwWidth = 0;
	DWORD			m_dwHeight = 0;
	float m_fDPI = 96.0f;

	UINT m_MultiSampleQualityLevels = 0;

	ID3D12Resource* m_pRenderTargets[SWAP_CHAIN_FRAME_COUNT] = {}; // R8G8B8A8_UNORM (SDR)
	ID3D12Resource* m_pDepthStencil = nullptr;
	ID3D12DescriptorHeap* m_pRTVHeap = nullptr;
	ID3D12DescriptorHeap* m_pDSVHeap = nullptr;
	ID3D12DescriptorHeap* m_pSRVHeap = nullptr;
	UINT	m_rtvDescriptorSize = 0;
	UINT	m_srvDescriptorSize = 0;
	UINT	m_hdrRTVDescriptorSize = 0;
	UINT	m_hdrSRVDescriptorSize = 0;
	UINT	m_dsvDescriptorSize = 0;
	UINT	m_dwSwapChainFlags = 0;
	UINT	m_uiRenderTargetIndex = 0;
	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;

	DWORD	m_dwCurContextIndex = 0;

	WCHAR	m_wchCurrentPathBackup[_MAX_PATH] = {};
	WCHAR	m_wchShaderPath[_MAX_PATH] = {};

	CCamera* m_pCamera = nullptr;

    //TODO: HDR rendering resources
	ID3D12Resource* m_pHDRRenderTarget[SWAP_CHAIN_FRAME_COUNT] = {};          // R16G16B16A16_FLOAT with MSAA
	ID3D12Resource* m_pHDRResolvedTarget[SWAP_CHAIN_FRAME_COUNT] = {};        // R16G16B16A16_FLOAT without MSAA
    ID3D12DescriptorHeap* m_pHDRRTVHeap = nullptr;         // For HDR RTVs
    ID3D12DescriptorHeap* m_pHDRResolvedHeap = nullptr;         // For HDR Resolve buffer
    ID3D12DescriptorHeap* m_pHDRResolvedSRVHeap = nullptr;         // For HDR SRVs
    
    // MSAA settings for HDR
    UINT m_dwMSAASampleCount = 4;                          // 4x MSAA
    UINT m_dwMSAAQuality = 0;
    UINT m_dwHDRColorFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
};
