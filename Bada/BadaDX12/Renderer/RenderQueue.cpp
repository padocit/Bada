#include "Pch.h"
#include "CommandListPool.h"
#include "BasicMeshObject.h"
#include "SpriteObject.h"
#include "D3D12Renderer.h"
#include "RenderQueue.h"

CRenderQueue::CRenderQueue()
{

}
CRenderQueue::~CRenderQueue()
{
	Cleanup();
}

BOOL CRenderQueue::Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxItemNum)
{
	m_pRenderer = pRenderer;
	m_dwMaxBufferSize = sizeof(RENDER_ITEM) * dwMaxItemNum;
	m_pBuffer = (char*)malloc(m_dwMaxBufferSize);
	memset(m_pBuffer, 0, m_dwMaxBufferSize);

	return TRUE;
}

BOOL CRenderQueue::Add(const RENDER_ITEM* pItem)
{
	BOOL bResult = FALSE;
	if (m_dwAllocatedSize + sizeof(RENDER_ITEM) > m_dwMaxBufferSize)
		goto lb_return;

	char* pDest = m_pBuffer + m_dwAllocatedSize;
	memcpy(pDest, pItem, sizeof(RENDER_ITEM));
	m_dwAllocatedSize += sizeof(RENDER_ITEM);
	m_dwItemCount++;
	bResult = TRUE;

lb_return:
	return bResult;
}

const RENDER_ITEM* CRenderQueue::Dispatch()
{
	const RENDER_ITEM* pItem = nullptr;
	if (m_dwReadBufferPos + sizeof(RENDER_ITEM) > m_dwAllocatedSize)
		goto lb_return;

	pItem = (const RENDER_ITEM*)(m_pBuffer + m_dwReadBufferPos);
	m_dwReadBufferPos += sizeof(RENDER_ITEM);

lb_return:
	return pItem;
}

DWORD CRenderQueue::Process(DWORD dwThreadIndex, 
	CCommandListPool* pCommandListPool, ID3D12CommandQueue* pCommandQueue,
	DWORD dwProcessCountPerCommandList, 
	D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, 
	const D3D12_VIEWPORT* pViewport, const D3D12_RECT* pScissorRect)
{
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();

	ID3D12GraphicsCommandList* ppCommandList[64] = {};
	DWORD dwCommandListCount = 0;

	ID3D12GraphicsCommandList* pCommandList = nullptr;
	DWORD dwProcessedCount = 0;
	DWORD dwProcessedCountPerCommandList = 0;
	const RENDER_ITEM* pItem = nullptr;

	while (pItem = Dispatch())
	{
		pCommandList = pCommandListPool->GetCurrentCommandList();
		pCommandList->RSSetViewports(1, pViewport);
		pCommandList->RSSetScissorRects(1, pScissorRect);
		pCommandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);

		switch (pItem->type)
		{
		case RENDER_ITEM_TYPE_MESH_OBJ:
			{
				CBasicMeshObject* pMeshObj = (CBasicMeshObject*)pItem->pObjHandle;
				pMeshObj->Draw(dwThreadIndex, pCommandList, &pItem->meshObjParam.matWorld);
			}
			break;
		case RENDER_ITEM_TYPE_SPRITE:
			{
				CSpriteObject* pSpriteObj = (CSpriteObject*)pItem->pObjHandle;
				TEXTURE_HANDLE* pTexureHandle = (TEXTURE_HANDLE*)pItem->spriteParam.pTexHandle;
				float Z = pItem->spriteParam.Z;

				if (pTexureHandle)
				{
					XMFLOAT2 Pos = { (float)pItem->spriteParam.iPosX, (float)pItem->spriteParam.iPosY };
					XMFLOAT2 Scale = { pItem->spriteParam.fScaleX, pItem->spriteParam.fScaleY };

					const RECT* pRect = nullptr;
					if (pItem->spriteParam.bUseRect)
					{
						pRect = &pItem->spriteParam.rect;
					}

					if (pTexureHandle->pUploadBuffer)
					{
						if (pTexureHandle->bUpdated)
						{
							UpdateTexture(pD3DDevice, pCommandList, pTexureHandle->pTexResource, pTexureHandle->pUploadBuffer);
						}
						else
						{
							int a = 0;
						}
						pTexureHandle->bUpdated = FALSE;
					}
					pSpriteObj->DrawWithTex(dwThreadIndex, pCommandList, &Pos, &Scale, pRect, Z, pTexureHandle);
				}
				else
				{
					CSpriteObject* pSpriteObj = (CSpriteObject*)pItem->pObjHandle;
					XMFLOAT2 Pos = { (float)pItem->spriteParam.iPosX, (float)pItem->spriteParam.iPosY };
					XMFLOAT2 Scale = { pItem->spriteParam.fScaleX, pItem->spriteParam.fScaleY };

					pSpriteObj->Draw(dwThreadIndex, pCommandList, &Pos, &Scale, Z);

				}
			}
			break;
		default:
			__debugbreak();
		}

		dwProcessedCount++;
		dwProcessedCountPerCommandList++;
		if (dwProcessedCountPerCommandList > dwProcessCountPerCommandList)
		{
			//pCommandListPool->CloseAndExecute(pCommandQueue);
			pCommandListPool->Close();
			ppCommandList[dwCommandListCount] = pCommandList;
			dwCommandListCount++;
			pCommandList = nullptr;
			dwProcessedCountPerCommandList = 0;
		}
	}
	// ���� ������������ ó��
	if (dwProcessedCountPerCommandList)
	{
		pCommandListPool->Close();
		ppCommandList[dwCommandListCount] = pCommandList;
		dwCommandListCount++;
		pCommandList = nullptr;
		dwProcessedCountPerCommandList = 0;
	}

	if (dwCommandListCount)
	{
		pCommandQueue->ExecuteCommandLists(dwCommandListCount, (ID3D12CommandList**)ppCommandList);
	}
	m_dwItemCount = 0;
	return dwProcessedCount;
}

void CRenderQueue::Reset()
{
	m_dwAllocatedSize = 0;
	m_dwReadBufferPos = 0;
}

void CRenderQueue::Cleanup()
{
	if (m_pBuffer)
	{
		free(m_pBuffer);
		m_pBuffer = nullptr;
	}
}