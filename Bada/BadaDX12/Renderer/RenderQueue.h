#pragma once

enum RENDER_ITEM_TYPE
{
	RENDER_ITEM_TYPE_MESH_OBJ,
	RENDER_ITEM_TYPE_SPRITE,
	RENDER_ITEM_TYPE_SKYBOX,
};

struct RENDER_MESH_OBJ_PARAM
{ 
	XMMATRIX matWorld;
};

struct RENDER_SPRITE_PARAM
{
	int iPosX;
	int iPosY;
	float fScaleX;
	float fScaleY;
	RECT rect;
	BOOL bUseRect;
	float Z;
	void* pTexHandle;
};

struct RENDER_ITEM
{
	RENDER_ITEM_TYPE type;
	void* pObjHandle;
	union
	{
		RENDER_MESH_OBJ_PARAM meshObjParam;
		RENDER_SPRITE_PARAM spriteParam;
		RENDER_MESH_OBJ_PARAM skyboxParam;
	};
};

class CCommandListPool;
class CD3D12Renderer;

class CRenderQueue
{
public:
	CRenderQueue();
	~CRenderQueue();

	BOOL Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxItemNum);
	BOOL Add(const RENDER_ITEM* pItem);
	DWORD Process(DWORD dwThreadIndex, CCommandListPool* pCommandListPool, ID3D12CommandQueue* pCommandQueue, DWORD dwProcessCountPerCommandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect);
	void Reset();

private:
	const RENDER_ITEM* Dispatch();
	void Cleanup();

private:
	CD3D12Renderer* m_pRenderer = nullptr;
	char* m_pBuffer = nullptr;
	DWORD m_dwMaxBufferSize = 0;
	DWORD m_dwAllocatedSize = 0;
	DWORD m_dwReadBufferPos = 0;
	DWORD m_dwItemCount = 0;
};

