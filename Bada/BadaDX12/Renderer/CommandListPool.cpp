#include "Pch.h"
#include "LinkedList.h"
#include "CommandListPool.h"

CCommandListPool::CCommandListPool()
{

}
CCommandListPool::~CCommandListPool()
{
	Cleanup();
}

BOOL CCommandListPool::Initialize(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, DWORD dwMaxCommandListNum)
{
	if (dwMaxCommandListNum < 2)
		__debugbreak();

	m_dwMaxCmdListNum = dwMaxCommandListNum;

	m_pD3DDevice = pDevice;

	return TRUE;
}

BOOL CCommandListPool::AddCmdList()
{
	BOOL bResult = FALSE;
	COMMAND_LIST* pCmdList = nullptr;

	ID3D12CommandAllocator* pDirectCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* pDirectCommandList = nullptr;

	if (m_dwTotalCmdNum >= m_dwMaxCmdListNum)
	{
	#ifdef _DEBUG
		__debugbreak();
	#endif
		goto lb_return;
	}

	HRESULT hr = m_pD3DDevice->CreateCommandAllocator(m_CommandListType,
		IID_PPV_ARGS(&pDirectCommandAllocator));
	if (FAILED(hr))
	{
	#ifdef _DEBUG
		hr = m_pD3DDevice->GetDeviceRemovedReason();
		__debugbreak();
	#endif
		goto lb_return;
	}

	if (FAILED(m_pD3DDevice->CreateCommandList(0, m_CommandListType, pDirectCommandAllocator, 
		nullptr, IID_PPV_ARGS(&pDirectCommandList))))
	{
	#ifdef _DEBUG
		__debugbreak();
	#endif
		pDirectCommandAllocator->Release();
		pDirectCommandAllocator = nullptr;
		goto lb_return;
	}
	pCmdList = new COMMAND_LIST;
	memset(pCmdList, 0, sizeof(COMMAND_LIST));
	pCmdList->Link.pItem = pCmdList;
	pCmdList->pDirectCommandAllocator = pDirectCommandAllocator;
	pCmdList->pDirectCommandList = pDirectCommandList;
	m_dwTotalCmdNum++;

	LinkToLinkedListFIFO(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->Link);
	m_dwAvailableCmdNum++;
	bResult = TRUE;

lb_return:
	return bResult;
}

COMMAND_LIST* CCommandListPool::AllocCmdList()
{
	COMMAND_LIST* pCmdList = nullptr;

	if (!m_pAvailableCmdLinkHead)
	{
		if (!AddCmdList())
		{
			goto lb_return;
		}
	}

	pCmdList = (COMMAND_LIST*)m_pAvailableCmdLinkHead->pItem;

	UnLinkFromLinkedList(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->Link);
	m_dwAvailableCmdNum--;

	LinkToLinkedListFIFO(&m_pAllocatedCmdLinkHead, &m_pAllocatedCmdLinkTail, &pCmdList->Link);
	m_dwAllocatedCmdNum++;

lb_return:
	return pCmdList;
}

ID3D12GraphicsCommandList* CCommandListPool::GetCurrentCommandList()
{
	ID3D12GraphicsCommandList* pCommandList = nullptr;
	if (!m_pCurCmdList)
	{
		m_pCurCmdList = AllocCmdList();
		if (!m_pCurCmdList)
		{
			__debugbreak();
		}
	}
	return m_pCurCmdList->pDirectCommandList;
}
void CCommandListPool::Close()
{
	if (!m_pCurCmdList)
		__debugbreak();

	if (m_pCurCmdList->bClosed)
		__debugbreak();

	if (FAILED(m_pCurCmdList->pDirectCommandList->Close()))
		__debugbreak();

	m_pCurCmdList->bClosed = TRUE;
	m_pCurCmdList = nullptr;
}

void CCommandListPool::CloseAndExecute(ID3D12CommandQueue* pCommandQueue)
{
	if (!m_pCurCmdList)
		__debugbreak();

	if (m_pCurCmdList->bClosed)
		__debugbreak();

	if (FAILED(m_pCurCmdList->pDirectCommandList->Close()))
		__debugbreak();

	m_pCurCmdList->bClosed = TRUE;

	pCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList**)&m_pCurCmdList->pDirectCommandList);
	m_pCurCmdList = nullptr;
}

void CCommandListPool::Reset()
{
	while (m_pAllocatedCmdLinkHead)
	{
		COMMAND_LIST* pCmdList = (COMMAND_LIST*)m_pAllocatedCmdLinkHead->pItem;

		if (FAILED(pCmdList->pDirectCommandAllocator->Reset()))
			__debugbreak();

		if (FAILED(pCmdList->pDirectCommandList->Reset(pCmdList->pDirectCommandAllocator, nullptr)))
			__debugbreak();

		pCmdList->bClosed = FALSE;

		UnLinkFromLinkedList(&m_pAllocatedCmdLinkHead, &m_pAllocatedCmdLinkTail, &pCmdList->Link);
		m_dwAllocatedCmdNum--;

		LinkToLinkedListFIFO(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->Link);
		m_dwAvailableCmdNum++;
	}
}

void CCommandListPool::Cleanup()
{
	Reset();

	while (m_pAvailableCmdLinkHead)
	{
		COMMAND_LIST* pCmdList = (COMMAND_LIST*)m_pAvailableCmdLinkHead->pItem;
		pCmdList->pDirectCommandList->Release();
		pCmdList->pDirectCommandList = nullptr;

		pCmdList->pDirectCommandAllocator->Release();
		pCmdList->pDirectCommandAllocator = nullptr;
		m_dwTotalCmdNum--;

		UnLinkFromLinkedList(&m_pAvailableCmdLinkHead, &m_pAvailableCmdLinkTail, &pCmdList->Link);
		m_dwAvailableCmdNum--;

		delete pCmdList;
	}
}
