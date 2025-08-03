#pragma once

class CD3D12Renderer;
class CFontManager
{
public:
	CFontManager();
	~CFontManager();

	BOOL Initialize(CD3D12Renderer* pRenderer, ID3D12CommandQueue* pCommandQueue, UINT width, UINT height, BOOL bEnableDebugLayer);
	FONT_HANDLE* CreateFontObject(const WCHAR* wchFontFamilyName, float fFontSize);
	void		 DeleteFontObject(FONT_HANDLE* pFontHandle);
	BOOL		 WriteTextToBitmap(BYTE* pDestImage, UINT destWidth, UINT destHeight, UINT destPitch, int* piOutWidth, int* piOutHeight, FONT_HANDLE* pFontHandle, const WCHAR* wchString, DWORD dwLen);
	
private:
	BOOL CreateD2D(ID3D12Device* pD3DDevice, ID3D12CommandQueue* pCommandQueue, BOOL bEnableDebugLayer);
	BOOL CreateDWrite(ID3D12Device* pD3DDevice, UINT texWidth, UINT texHeight, float fDPI);
	BOOL CreateBitmapFromText(int* piOutWidth, int* piOutHeight, IDWriteTextFormat* pTextFormat, const WCHAR* wchString, DWORD dwLen);
	void CleanupDWrite();
	void CleanupD2D();
	void Cleanup();

private:
	CD3D12Renderer* m_pRenderer = nullptr;
	ID2D1Device2* m_pD2DDevice = nullptr;
	ID2D1DeviceContext2* m_pD2DDeviceContext = nullptr;

	ID2D1Bitmap1* m_pD2DTargetBitmap = nullptr;
	ID2D1Bitmap1* m_pD2DTargetBitmapRead = nullptr;
	IDWriteFontCollection1* m_pFontCollection = nullptr;
	ID2D1SolidColorBrush* m_pWhiteBrush = nullptr;

	IDWriteFactory5* m_pDWFactory = nullptr;
	DWRITE_LINE_METRICS* m_pLineMetrics = nullptr;
	DWORD m_dwMaxLineMetricsNum = 0;
	UINT m_D2DBitmapWidth = 0;
	UINT m_D2DBitmapHeight = 0;
};

