#pragma once

enum SPRITE_DESCRIPTOR_INDEX
{
	SPRITE_DESCRIPTOR_INDEX_CBV = 0,
	SPRITE_DESCRIPTOR_INDEX_TEX = 1
};

class CD3D12Renderer;
class CSpriteObject : public ISprite
{
public:
	// derived from IUnknown
	STDMETHODIMP			QueryInterface(REFIID, void** ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

public:
	static const UINT DESCRIPTOR_COUNT_FOR_DRAW = 2; // | Constant Buffer | Tex |

	CSpriteObject();
	~CSpriteObject();

	BOOL Initialize(CD3D12Renderer* pRenderer);
	BOOL Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchTexFileName, const RECT* pRect);
	void DrawWithTex(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, const RECT* pRect, float Z, TEXTURE_HANDLE* pTexHandle);
	void Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMFLOAT2* pPos, const XMFLOAT2* pScale, float Z);

private:
	// Shared by all CSpriteObject instances.
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD m_dwInitRefCount;

	// Vertex data
	static ID3D12Resource* m_pVertexBuffer;
	static D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

	// Index data
	static ID3D12Resource* m_pIndexBuffer;
	static D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	BOOL InitCommonResources();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();
	BOOL InitMesh();

	void Cleanup();

private:
	DWORD m_dwRefCount = 1;
	TEXTURE_HANDLE* m_pTexHandle = nullptr;
	CD3D12Renderer* m_pRenderer = nullptr;
	RECT m_Rect = {};
	XMFLOAT2 m_Scale = { 1.0f, 1.0f };

	DWORD m_dwTriGroupCount = 0;
	DWORD m_dwMaxTriGroupCount = 0;
};