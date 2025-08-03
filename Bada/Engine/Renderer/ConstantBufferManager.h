#pragma once

#include "Typedef.h"

class CSimpleConstantBufferPool;

class CConstantBufferManager
{
public:
	CConstantBufferManager();
	~CConstantBufferManager();

	BOOL Initialize(ID3D12Device* pD3DDevice, DWORD dwMaxCBVNum);
	void Reset();
	CSimpleConstantBufferPool* GetConstantBufferPool(CONSTANT_BUFFER_TYPE);

private:
	CSimpleConstantBufferPool* m_ppConstantBufferPool[CONSTANT_BUFFER_TYPE_COUNT] = {};
};

