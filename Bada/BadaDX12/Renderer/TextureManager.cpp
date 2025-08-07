#include "Pch.h"
#include "Typedef.h"
#include "HashTable.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "SingleDescriptorAllocator.h"
#include "TextureManager.h"

CTextureManager::CTextureManager()
{

}

CTextureManager::~CTextureManager()
{
	Cleanup();
}

BOOL CTextureManager::Initialize(CD3D12Renderer* pRenderer, DWORD dwMaxBucketNum, DWORD dwMaxFileNum)
{
	m_pRenderer = pRenderer;
	m_pResourceManager = pRenderer->INL_GetResourceManager();

	m_pHashTable = new CHashTable;
	m_pHashTable->Initialize(dwMaxBucketNum, _MAX_PATH * sizeof(WCHAR), dwMaxFileNum);

	return TRUE;
}

TEXTURE_HANDLE* CTextureManager::CreateTextureFromFile(const WCHAR* wchFileName, BOOL bIsCubeMap)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	ID3D12Resource* pTexResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
	D3D12_RESOURCE_DESC	desc = {};
	TEXTURE_HANDLE* pTexHandle = nullptr;

	DWORD dwFileNameLen = (DWORD)wcslen(wchFileName);
	DWORD dwKeySize = dwFileNameLen * sizeof(WCHAR);
	if (m_pHashTable->Select((void**)&pTexHandle, 1, wchFileName, dwKeySize))
	{
		pTexHandle->dwRefCount++;
	}
	else
	{
		if (m_pResourceManager->CreateTextureFromFile(&pTexResource, &desc, wchFileName, bIsCubeMap))
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = desc.Format;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;				

			if (bIsCubeMap)
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
				SRVDesc.TextureCube.MipLevels = desc.MipLevels;
				SRVDesc.TextureCube.MostDetailedMip = 0;
				SRVDesc.TextureCube.ResourceMinLODClamp = 0;
			}
			else
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				SRVDesc.Texture2D.MipLevels = desc.MipLevels;
			}


			if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
			{
				pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

				pTexHandle = AllocTextureHandle();
				pTexHandle->pTexResource = pTexResource;
				pTexHandle->bFromFile = TRUE;
				pTexHandle->srv = srv;

				pTexHandle->pSearchHandle = 
					m_pHashTable->Insert((void*)pTexHandle, wchFileName, dwKeySize);

				if (!pTexHandle->pSearchHandle)
					__debugbreak();
			}
			else
			{
				pTexResource->Release();
				pTexResource = nullptr;
			}
		}
	}
	return pTexHandle;
}

TEXTURE_HANDLE* CTextureManager::CreateDynamicTexture(UINT texWidth, UINT texHeight)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();
	TEXTURE_HANDLE* pTexHandle = nullptr;

	ID3D12Resource* pTexResource = nullptr;
	ID3D12Resource* pUploadBuffer = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};


	DXGI_FORMAT TexFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_pResourceManager->CreateTexturePair(&pTexResource, &pUploadBuffer, 
		texWidth, texHeight, TexFormat))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = TexFormat;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
		{
			pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

			pTexHandle = AllocTextureHandle();
			pTexHandle->pTexResource = pTexResource;
			pTexHandle->pUploadBuffer = pUploadBuffer;
			pTexHandle->srv = srv;
		}
		else
		{
			pTexResource->Release();
			pTexResource = nullptr;

			pUploadBuffer->Release();
			pUploadBuffer = nullptr;
		}
	}

	return pTexHandle;
}

TEXTURE_HANDLE* CTextureManager::CreateImmutableTexture(UINT texWidth, UINT texHeight, DXGI_FORMAT format, const BYTE* pInitImage)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();
	TEXTURE_HANDLE* pTexHandle = nullptr;

	ID3D12Resource* pTexResource = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};

	if (m_pResourceManager->CreateTexture(&pTexResource, texWidth, texHeight, format, pInitImage))
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = format;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		if (pSingleDescriptorAllocator->AllocDescriptorHandle(&srv))
		{
			pD3DDevice->CreateShaderResourceView(pTexResource, &SRVDesc, srv);

			pTexHandle = AllocTextureHandle();
			pTexHandle->pTexResource = pTexResource;
			pTexHandle->srv = srv;
		}
		else
		{
			pTexResource->Release();
			pTexResource = nullptr;
		}
	}

	return pTexHandle;
}

void CTextureManager::DeleteTexture(TEXTURE_HANDLE* pTexHandle)
{
	FreeTextureHandle(pTexHandle);
}

TEXTURE_HANDLE* CTextureManager::AllocTextureHandle()
{
	TEXTURE_HANDLE* pTexHandle = new TEXTURE_HANDLE;
	memset(pTexHandle, 0, sizeof(TEXTURE_HANDLE));
	pTexHandle->Link.pItem = pTexHandle;
	LinkToLinkedListFIFO(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->Link);
	pTexHandle->dwRefCount = 1;
	return pTexHandle;
}

DWORD CTextureManager::FreeTextureHandle(TEXTURE_HANDLE* pTexHandle)
{
	ID3D12Device* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	if (!pTexHandle->dwRefCount)
		__debugbreak();

	DWORD ref_count = --pTexHandle->dwRefCount;
	if (!ref_count)
	{
		if (pTexHandle->pTexResource)
		{
			pTexHandle->pTexResource->Release();
			pTexHandle->pTexResource = nullptr;
		}
		if (pTexHandle->pUploadBuffer)
		{
			pTexHandle->pUploadBuffer->Release();
			pTexHandle->pUploadBuffer = nullptr;
		}
		if (pTexHandle->srv.ptr)
		{
			pSingleDescriptorAllocator->FreeDescriptorHandle(pTexHandle->srv);
			pTexHandle->srv = {};
		}

		if (pTexHandle->pSearchHandle)
		{
			m_pHashTable->Delete(pTexHandle->pSearchHandle);
			pTexHandle->pSearchHandle = nullptr;
		}
		UnLinkFromLinkedList(&m_pTexLinkHead, &m_pTexLinkTail, &pTexHandle->Link);

		delete pTexHandle;
	}
	else
	{
		int a = 0;
	}
	return ref_count;
}

void CTextureManager::Cleanup()
{
	if (m_pTexLinkHead)
	{
		// texture resource leak!!!
		__debugbreak();
	}
	if (m_pHashTable)
	{
		delete m_pHashTable;
		m_pHashTable = nullptr;
	}
}
