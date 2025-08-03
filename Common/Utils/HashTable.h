#pragma once

struct BUCKET_DESC
{
	SORT_LINK*	pBucketLinkHead;
	SORT_LINK*	pBucketLinkTail;
	DWORD		dwLinkNum;
};

struct VB_BUCKET
{
	// Header
	const void*		pItem;
	BUCKET_DESC*	pBucketDesc;
	SORT_LINK		sortLink;
	DWORD			dwSize;
	// Data start address
	char			pKeyData[1];
};


class CHashTable
{
public:
	CHashTable();
	~CHashTable();

	BOOL	Initialize(DWORD dwMaxBucketNum, DWORD dwMaxKeySize, DWORD dwMaxItemNum);
	DWORD	Select(void** ppOutItemList, DWORD dwMaxItemNum, const void* pKeyData, DWORD dwSize);
	void*	Insert(const void* pItem, const void* pKeyData, DWORD dwSize);
	void	Delete(const void* pSearchHandle);
	DWORD	GetMaxBucketNum() const;
	void	DeleteAll();
	DWORD	GetAllItems(void** ppOutItemList, DWORD dwMaxItemNum, BOOL* pbOutInsufficient) const;
	DWORD	GetKeyPtrAndSize(void** ppOutKeyPtr, const void* pSearchHandle) const;
	DWORD	GetItemNum() const;
	void	ResourceCheck() const;

	void	Cleanup();

private:
	DWORD	CreateKey(const void* pData, DWORD dwSize, DWORD dwBucketNum);

private:
	BUCKET_DESC*	m_pBucketDescTable = nullptr;
	DWORD	m_dwMaxBucketNum = 0;
	DWORD	m_dwMaxKeyDataSize = 0;
	DWORD	m_dwItemNum = 0;


};


