#pragma once

class CD3D12Renderer;
class CSkyboxMeshObject : public IMeshObject
{
public:
	// Derived from IUnknown
	STDMETHODIMP			QueryInterface(REFIID, void** ppv);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	// Shared by all BasicMeshObject instances.
	static const UINT DESCRIPTOR_COUNT_PER_OBJ = 1;	// | Constant Buffer
	static const UINT DESCRIPTOR_COUNT_PER_TRI_GROUP = 1; // | SRV(tex)
	static const UINT MAX_TRI_GROUP_COUNT_PER_OBJ = 8;
	static const UINT MAX_DESCRIPTOR_COUNT_FOR_DRAW = DESCRIPTOR_COUNT_PER_OBJ + (MAX_TRI_GROUP_COUNT_PER_OBJ * DESCRIPTOR_COUNT_PER_TRI_GROUP);

	CSkyboxMeshObject();
	~CSkyboxMeshObject();

	BOOL Initialize(CD3D12Renderer* pRenderer);
	void Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, const XMMATRIX* pMatWorld);

	// Derived from IMeshObject
	BOOL __stdcall BeginCreateMesh(const BasicVertex* pVertexList, DWORD dwVertexNum, DWORD dwTriGroupCount);   // Create Vertex Buffer
	BOOL __stdcall InsertTriGroup(const WORD* pIndexList, DWORD dwTriCount, const WCHAR* wchTexFileName); // Create Index Buffer
	void __stdcall EndCreateMesh();

private:
	static ID3D12RootSignature* m_pRootSignature;
	static ID3D12PipelineState* m_pPipelineState;
	static DWORD	m_dwInitRefCount;

	BOOL InitCommonResources();
	void CleanupSharedResources();

	BOOL InitRootSignature();
	BOOL InitPipelineState();

	void DeleteTriGroup(INDEXED_TRI_GROUP* pTriGroup);
	void Cleanup();

private:
	DWORD m_dwRefCount = 1;
	CD3D12Renderer* m_pRenderer = nullptr;

	// vertex data
	ID3D12Resource* m_pVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};

	// index data
	INDEXED_TRI_GROUP* m_pTriGroupList = nullptr;
	DWORD m_dwTriGroupCount = 0;
	DWORD m_dwMaxTriGroupCount = 0;
};
