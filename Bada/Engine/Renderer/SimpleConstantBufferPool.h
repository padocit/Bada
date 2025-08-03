#pragma once

struct CB_CONTAINER
{
	D3D12_CPU_DESCRIPTOR_HANDLE CBVHandle;
	D3D12_GPU_VIRTUAL_ADDRESS pGPUMemAddr;
	UINT8* pSystemMemAddr;
};

class CSimpleConstantBufferPool
{
public:
	CSimpleConstantBufferPool();
	~CSimpleConstantBufferPool();

	BOOL Initialize(ID3D12Device* pD3DDevice, CONSTANT_BUFFER_TYPE type, UINT SizePerCBV, UINT MaxCBVNum);

	CB_CONTAINER* Alloc();
	void Reset();

private:
	void Cleanup();

private:
	CB_CONTAINER* m_pCBContainerList = nullptr;
	CONSTANT_BUFFER_TYPE m_ConstantBufferType = (CONSTANT_BUFFER_TYPE)0;
	ID3D12DescriptorHeap* m_pCBVHeap = nullptr;
	ID3D12Resource* m_pResource = nullptr;
	UINT8* m_pSystemMemAddr = nullptr;
	UINT m_SizePerCBV = 0;
	UINT m_MaxCBVNum = 0;
	UINT m_AllocatedCBVNum = 0;
};
