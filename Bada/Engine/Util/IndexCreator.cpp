#include "Pch.h"
#include "IndexCreator.h"

CIndexCreator::CIndexCreator()
{
}

CIndexCreator::~CIndexCreator()
{
	Check();
	Cleanup();
}

BOOL CIndexCreator::Initialize(DWORD dwNum)
{
	m_pdwIndexTable = new DWORD[dwNum];
	memset(m_pdwIndexTable, 0, sizeof(DWORD) * dwNum);
	m_dwMaxNum = dwNum;

	for (DWORD i = 0; i < m_dwMaxNum; i++)
	{
		m_pdwIndexTable[i] = i;
	}

	return TRUE;
}

DWORD CIndexCreator::Alloc()
{
	DWORD dwResult = -1;

	if (m_dwAllocatedCount >= m_dwMaxNum)
	{
		goto lb_return;
	}

	dwResult = m_pdwIndexTable[m_dwAllocatedCount];
	m_dwAllocatedCount++;

lb_return:
	return dwResult;
}

void CIndexCreator::Free(DWORD dwIndex)
{
	if (!m_dwAllocatedCount)
	{
		__debugbreak();
	}
	m_dwAllocatedCount--;
	m_pdwIndexTable[m_dwAllocatedCount] = dwIndex;
}

void CIndexCreator::Cleanup()
{
	if (m_pdwIndexTable)
	{
		delete[] m_pdwIndexTable;
		m_pdwIndexTable = nullptr;
	}
}

void CIndexCreator::Check()
{
	if (m_dwAllocatedCount)
	{
		__debugbreak();
	}
}