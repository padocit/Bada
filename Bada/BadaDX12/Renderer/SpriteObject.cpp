#include "Pch.h"
#include "Typedef.h"
#include "D3DUtils.h"
#include "D3D12Renderer.h"
#include "D3D12ResourceManager.h"
#include "DescriptorPool.h"
#include "SimpleConstantBufferPool.h"
#include "SingleDescriptorAllocator.h"
#include "SpriteObject.h"

#include <d3dcompiler.h>
#include <d3dx12.h>
#include <DirectXMath.h>

using namespace DirectX;

ID3D12RootSignature* CSpriteObject::m_pRootSignature = nullptr;
ID3D12PipelineState* CSpriteObject::m_pPipelineState = nullptr;

ID3D12Resource* CSpriteObject::m_pVertexBuffer = nullptr;
D3D12_VERTEX_BUFFER_VIEW CSpriteObject::m_VertexBufferView = {};

ID3D12Resource* CSpriteObject::m_pIndexBuffer = nullptr;
D3D12_INDEX_BUFFER_VIEW CSpriteObject::m_IndexBufferView = {};

DWORD CSpriteObject::m_dwInitRefCount = 0;

STDMETHODIMP CSpriteObject::QueryInterface(REFIID refiid, void** ppv)
{
	return E_NOINTERFACE; //NOTE: no-op
}
STDMETHODIMP_(ULONG) CSpriteObject::AddRef()
{
	m_dwRefCount++;
	return m_dwRefCount;

}
STDMETHODIMP_(ULONG) CSpriteObject::Release()
{
	DWORD	ref_count = --m_dwRefCount;
	if (!m_dwRefCount)
		delete this;

	return ref_count;
}

CSpriteObject::CSpriteObject()
{
}

CSpriteObject::~CSpriteObject()
{
	m_pRenderer->EnsureCompleted();
	Cleanup();
}

BOOL CSpriteObject::Initialize(CD3D12Renderer* pRenderer)
{
	m_pRenderer = pRenderer;

	BOOL bResult = InitCommonResources();
	return bResult;
}

BOOL CSpriteObject::Initialize(CD3D12Renderer* pRenderer, const WCHAR* wchTexFileName, 
	const RECT* pRect)
{
	m_pRenderer = pRenderer;

	BOOL bResult = InitCommonResources();
	if (bResult)
	{
		UINT TexWidth = 1;
		UINT TexHeight = 1;
		m_pTexHandle = (TEXTURE_HANDLE*)m_pRenderer->CreateTextureFromFile(wchTexFileName);
		if (m_pTexHandle)
		{
			D3D12_RESOURCE_DESC desc = m_pTexHandle->pTexResource->GetDesc();
			TexWidth = (UINT)desc.Width;
			TexHeight = (UINT)desc.Height;
		}
		if (pRect)
		{
			m_Rect = *pRect;
			m_Scale.x = (float)(m_Rect.right - m_Rect.left) / (float)TexWidth;
			m_Scale.y = (float)(m_Rect.bottom - m_Rect.top) / (float)TexHeight;
		}
		else
		{
			if (m_pTexHandle)
			{
				D3D12_RESOURCE_DESC desc = m_pTexHandle->pTexResource->GetDesc();
				m_Rect.left = 0;
				m_Rect.top = 0;
				m_Rect.right = (LONG)desc.Width;
				m_Rect.bottom = (LONG)desc.Height;
			}
		}
	}
	return bResult;
}

BOOL CSpriteObject::InitCommonResources()
{
	if (m_dwInitRefCount)
		goto lb_true;

	InitRootSignature();
	InitPipelineState();
	InitMesh();

lb_true:
	m_dwInitRefCount++;
	return m_dwInitRefCount;
}

BOOL CSpriteObject::InitRootSignature()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	ID3DBlob* pSignature = nullptr;
	ID3DBlob* pError = nullptr;

	CD3DX12_DESCRIPTOR_RANGE ranges[2] = {};
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0); // b0 : Constant Buffer View
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0 : Shader Resource View (Texture)

	CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
	rootParameters[0].InitAsDescriptorTable(_countof(ranges), ranges, D3D12_SHADER_VISIBILITY_ALL);

	// default sampler
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	SetDefaultSamplerDesc(&sampler, 0);
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;

	// Allow input layout and deny uneccessary access to certain pipeline stages.
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	// Create an root signature.
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	//rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pSignature, &pError)))
	{
		__debugbreak();
	}

	if (FAILED(pD3DDevice->CreateRootSignature(0, pSignature->GetBufferPointer(), pSignature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature))))
	{
		__debugbreak();
	}
	if (pSignature)
	{
		pSignature->Release();
		pSignature = nullptr;
	}
	if (pError)
	{
		pError->Release();
		pError = nullptr;
	}
	return TRUE;

}

BOOL CSpriteObject::InitPipelineState()
{
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();

	ID3DBlob* pVertexShader = nullptr;
	ID3DBlob* pPixelShader = nullptr;

#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	m_pRenderer->SetCurrentPathForShader();

	ID3DBlob* pErrorBlob = nullptr;
	if (FAILED(D3DCompileFromFile(L"shSprite.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &pVertexShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	if (FAILED(D3DCompileFromFile(L"shSprite.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pPixelShader, &pErrorBlob)))
	{
		if (pErrorBlob != nullptr)
		{
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
			pErrorBlob->Release();
		}
		__debugbreak();
	}
	m_pRenderer->RestoreCurrentPath();

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,	0, 28,	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_pRootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(pVertexShader->GetBufferPointer(), pVertexShader->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pPixelShader->GetBufferPointer(), pPixelShader->GetBufferSize());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	psoDesc.SampleDesc.Count = 1;
	if (FAILED(pD3DDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState))))
	{
		__debugbreak();
	}

	if (pVertexShader)
	{
		pVertexShader->Release();
		pVertexShader = nullptr;
	}
	if (pPixelShader)
	{
		pPixelShader->Release();
		pPixelShader = nullptr;
	}
	return TRUE;
}

BOOL CSpriteObject::InitMesh()
{
	BOOL bResult = FALSE;
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CD3D12ResourceManager* pResourceManager = m_pRenderer->INL_GetResourceManager();
	CSingleDescriptorAllocator* pSingleDescriptorAllocator = m_pRenderer->INL_GetSingleDescriptorAllocator();

	// Define the geometry for a Quad. (Screen coordinates)
	BasicVertex vertices[] =
	{
		{ { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
	};

	WORD indices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	const UINT vertexBufferSize = sizeof(vertices);

	if (FAILED(pResourceManager->CreateVertexBuffer(sizeof(BasicVertex), (DWORD)_countof(vertices),
		&m_VertexBufferView, &m_pVertexBuffer, vertices)))
	{
		__debugbreak();
		goto lb_return;
	}

	if (FAILED(pResourceManager->CreateIndexBuffer((DWORD)_countof(indices), 
		&m_IndexBufferView, &m_pIndexBuffer, indices)))
	{
		__debugbreak();
		goto lb_return;
	}
	bResult = TRUE;

lb_return:
	return bResult;

}

void CSpriteObject::Draw(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, 
	const XMFLOAT2* pPos, const XMFLOAT2* pScale, float Z)
{
	XMFLOAT2 scale = { m_Scale.x * pScale->x, m_Scale.y * pScale->y };
	DrawWithTex(dwThreadIndex, pCommandList, pPos, &scale, &m_Rect, Z, m_pTexHandle);
}

void CSpriteObject::DrawWithTex(DWORD dwThreadIndex, ID3D12GraphicsCommandList* pCommandList, 
	const XMFLOAT2* pPos, const XMFLOAT2* pScale, const RECT* pRect, float Z, TEXTURE_HANDLE* pTexHandle)
{
	// Pool new CBVs and descriptors every draw.
	ID3D12Device5* pD3DDevice = m_pRenderer->INL_GetD3DDevice();
	UINT srvDescriptorSize = m_pRenderer->INL_GetSrvDescriptorSize();
	CDescriptorPool* pDescriptorPool = m_pRenderer->INL_GetDescriptorPool(dwThreadIndex);
	ID3D12DescriptorHeap* pDescriptorHeap = pDescriptorPool->INL_GetDescriptorHeap();
	CSimpleConstantBufferPool* pConstantBufferPool = m_pRenderer->GetConstantBufferPool(CONSTANT_BUFFER_TYPE_SPRITE, dwThreadIndex);

	UINT TexWidth = 0;
	UINT TexHeight = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
	if (pTexHandle)
	{
		D3D12_RESOURCE_DESC desc = pTexHandle->pTexResource->GetDesc();
		TexWidth = (UINT)desc.Width;
		TexHeight = (UINT)desc.Height;
		srv = pTexHandle->srv;
	}

	RECT rect;
	if (!pRect)
	{
		rect.left = 0;
		rect.top = 0;
		rect.right = TexWidth;
		rect.bottom = TexHeight;
		pRect = &rect;
	}

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuDescriptorTable = {};
	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDescriptorTable = {};

	if (!pDescriptorPool->AllocDescriptorTable(&cpuDescriptorTable, &gpuDescriptorTable,
		DESCRIPTOR_COUNT_FOR_DRAW))
	{
		__debugbreak();
	}

	// Constant buffer
	CB_CONTAINER* pCB = pConstantBufferPool->Alloc();
	if (!pCB)
	{
		__debugbreak();
	}
	CONSTANT_BUFFER_SPRITE* pConstantBufferSprite = (CONSTANT_BUFFER_SPRITE*)pCB->pSystemMemAddr;

	pConstantBufferSprite->ScreenRes.x = (float)m_pRenderer->INL_GetScreenWidth();
	pConstantBufferSprite->ScreenRes.y = (float)m_pRenderer->INL_GetScreenHeight();
	pConstantBufferSprite->Pos = *pPos;
	pConstantBufferSprite->Scale = *pScale;
	pConstantBufferSprite->TexSize.x = (float)TexWidth;
	pConstantBufferSprite->TexSize.y = (float)TexHeight;
	pConstantBufferSprite->TexSamplePos.x = (float)pRect->left;
	pConstantBufferSprite->TexSamplePos.y = (float)pRect->top;
	pConstantBufferSprite->TexSampleSize.x = (float)(pRect->right - pRect->left);
	pConstantBufferSprite->TexSampleSize.y = (float)(pRect->bottom - pRect->top);
	pConstantBufferSprite->Z = Z;
	pConstantBufferSprite->Alpha = 1.0f;

	// RootSignature
	pCommandList->SetGraphicsRootSignature(m_pRootSignature);
	pCommandList->SetDescriptorHeaps(1, &pDescriptorHeap);

	// Descriptor table
	CD3DX12_CPU_DESCRIPTOR_HANDLE cbvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_CBV, srvDescriptorSize);
	pD3DDevice->CopyDescriptorsSimple(1, cbvDest, pCB->CBVHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	if (srv.ptr)
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE srvDest(cpuDescriptorTable, SPRITE_DESCRIPTOR_INDEX_TEX, srvDescriptorSize);
		pD3DDevice->CopyDescriptorsSimple(1, srvDest, srv, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	pCommandList->SetGraphicsRootDescriptorTable(0, gpuDescriptorTable);

	pCommandList->SetPipelineState(m_pPipelineState);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
	pCommandList->IASetIndexBuffer(&m_IndexBufferView);
	pCommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

void CSpriteObject::Cleanup()
{
	if (m_pTexHandle)
	{
		m_pRenderer->DeleteTexture(m_pTexHandle);
		m_pTexHandle = nullptr;
	}
	CleanupSharedResources();
}

void CSpriteObject::CleanupSharedResources()
{
	if (!m_dwInitRefCount)
		return;

	DWORD ref_count = --m_dwInitRefCount;
	if (!ref_count)
	{
		if (m_pRootSignature)
		{
			m_pRootSignature->Release();
			m_pRootSignature = nullptr;
		}
		if (m_pPipelineState)
		{
			m_pPipelineState->Release();
			m_pPipelineState = nullptr;
		}
		if (m_pVertexBuffer)
		{
			m_pVertexBuffer->Release();
			m_pVertexBuffer = nullptr;
		}
		if (m_pIndexBuffer)
		{
			m_pIndexBuffer->Release();
			m_pIndexBuffer = nullptr;
		}
	}
}
