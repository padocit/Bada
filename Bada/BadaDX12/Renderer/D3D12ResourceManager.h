#pragma once

class CD3D12ResourceManager {
public:
	CD3D12ResourceManager();
	~CD3D12ResourceManager();

	BOOL Initialize(ID3D12Device5* pD3DDevice);
	HRESULT CreateVertexBuffer(UINT SizePerVertex, DWORD dwVertexNum, D3D12_VERTEX_BUFFER_VIEW* pOutVertexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);
	HRESULT CreateIndexBuffer(DWORD dwIndexNum, D3D12_INDEX_BUFFER_VIEW* pOutIndexBufferView, ID3D12Resource** ppOutBuffer, void* pInitData);

	BOOL	CreateTexture(ID3D12Resource** ppOutResource, UINT Width, UINT Height, DXGI_FORMAT format, const BYTE* pInitImage);
	BOOL	CreateTextureFromFile(ID3D12Resource** ppOutResource, D3D12_RESOURCE_DESC* pOutDesc, const WCHAR* wchFileName, bool bIsCubeMap = false);
	BOOL	CreateTexturePair(ID3D12Resource** ppOutResource, ID3D12Resource** ppOutUploadBuffer, UINT width, UINT height, DXGI_FORMAT format);
	void	UpdateTextureForWrite(ID3D12Resource* pDestTexResource, ID3D12Resource* pSrcTexResource);

private:
	void CreateFence();
	void CleanupFence();
	void CreateCommandList();
	void CleanupCommandList();

	UINT64 Fence();
	void WaitForFenceValue();

	void Cleanup();

private:
	ID3D12Device5* m_pD3DDevice = nullptr;
	ID3D12CommandQueue* m_pCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pCommandList = nullptr;

	HANDLE	m_hFenceEvent = nullptr;
	ID3D12Fence* m_pFence = nullptr;
	UINT64	m_ui64FenceValue = 0;
};
