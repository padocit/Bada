#pragma once

struct COMMAND_LIST
{
	ID3D12CommandAllocator* pDirectCommandAllocator;
	ID3D12GraphicsCommandList* pDirectCommandList;
	SORT_LINK Link;
	BOOL bClosed;
};

class CCommandListPool
{
public:
	CCommandListPool();
	~CCommandListPool();

	BOOL Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, DWORD dwMaxCommandListNum);
	ID3D12GraphicsCommandList* GetCurrentCommandList();
	void Close();
	void CloseAndExecute(ID3D12CommandQueue* pCommandQueue);
	void Reset();

	DWORD GetTotalCmdListNum() const { return m_dwTotalCmdNum; }
	DWORD GetAllocatedCmdListNum() const { return m_dwAllocatedCmdNum; }
	DWORD GetAvailableCmdListNum() const { return m_dwAvailableCmdNum; }
	ID3D12Device* INL_GetD3DDevice() { return m_pD3DDevice; }
	
private:
	BOOL AddCmdList();
	COMMAND_LIST* AllocCmdList();
	void Cleanup();

private:
	ID3D12Device* m_pD3DDevice = nullptr;
	D3D12_COMMAND_LIST_TYPE m_CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
	DWORD m_dwAllocatedCmdNum = 0;
	DWORD m_dwAvailableCmdNum = 0;
	DWORD m_dwTotalCmdNum = 0;
	DWORD m_dwMaxCmdListNum = 0;
	COMMAND_LIST* m_pCurCmdList = nullptr;
	SORT_LINK* m_pAllocatedCmdLinkHead = nullptr;
	SORT_LINK* m_pAllocatedCmdLinkTail = nullptr;
	SORT_LINK* m_pAvailableCmdLinkHead = nullptr;
	SORT_LINK* m_pAvailableCmdLinkTail = nullptr;
};

