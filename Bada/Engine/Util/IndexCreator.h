#pragma once

class CIndexCreator
{
public:
	CIndexCreator();
	~CIndexCreator();

	BOOL Initialize(DWORD dwNum);
	
	DWORD Alloc();
	void Free(DWORD dwIndex);
	void Cleanup();
	void Check();

private:
	DWORD* m_pdwIndexTable = nullptr;
	DWORD  m_dwMaxNum = 0;
	DWORD  m_dwAllocatedCount = 0;
};