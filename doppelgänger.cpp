// doppelgänger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "doppelgänger.h"

HRESULT hr;
ID3D11Device* MeinDevice;
ID3D11DeviceContext* MeinContext;
IDXGIOutputDuplication* MeinDeskDupl;
ID3D11Texture2D* MeinAcquiredDesktopImage;
BYTE* MeinMetaDataBuffer = NULL;
UINT MeinMetaDataSize = 0;

int _tmain(int argc, _TCHAR* argv[])
{
	UINT DriverTypeIndex;
	IDXGIDevice* DxgiDevice = NULL;
	IDXGIAdapter* DxgiAdapter = NULL;

	DXGI_OUTPUT_DESC desc;
	UINT i = 0, t, BufSize;
	UINT dTop;
	IDXGIOutput * pOutput;

	IDXGIOutput* DxgiOutput = NULL;

	IDXGIOutput1* DxgiOutput1 = NULL;

	IDXGIResource* DesktopResource = NULL;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	BYTE* DirtyRects;

	UINT dirty;
	RECT* pRect;

	///////////////////////////////////////////////////////

	//guessing this must be null
	MeinAcquiredDesktopImage = NULL;


	_tprintf(_T("Hallo, welt!\n"));

	_tprintf(_T("Trying to create a DX11 Device...\n"));
	// Create device
	for (DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(NULL, DriverTypes[DriverTypeIndex], NULL, 0, FeatureLevels, NumFeatureLevels,
								D3D11_SDK_VERSION, &MeinDevice, &FeatureLevel, &MeinContext);
		if (SUCCEEDED(hr))
		{
			// Device creation success, no need to loop anymore
			break;
		}
	}
	if (FAILED(hr))
	{
		_tprintf(_T("Failed to create device in InitializeDx\n"));
		return 1;
	}
	_tprintf(_T("Gut!\n"));

	///////////////////////////////////////////////////////

	
	_tprintf(_T("Trying to get QI for DXGI Device...\n"));
	
	hr = MeinDevice->lpVtbl->QueryInterface(MeinDevice, &IID_IDXGIDevice, (void**) &DxgiDevice);
	if (FAILED(hr))
    {
		_tprintf(_T("Failed to get QI for DXGI Device\n"));
        return 1;
    }
	_tprintf(_T("Gut!\n"));

	//////////////////////////////////////////////////////////

	_tprintf(_T("Trying to get adapter for DXGI Device...\n"));
	
	hr = DxgiDevice->lpVtbl->GetParent(DxgiDevice, &IID_IDXGIAdapter, (void**) &DxgiAdapter);
	DxgiDevice->lpVtbl->Release(DxgiDevice);
    DxgiDevice = NULL;
    if (FAILED(hr))
    {
        _tprintf(_T("Failed to get parent DXGI Adapter\n"));
		return 1;
    }
	_tprintf(_T("Gut!\n"));
	
	////////////////////////////////////////////////////////////

	
	memset(&desc, 0, sizeof(desc));

	
	
	_tprintf(_T("\nLooping through ouputs on DXGI adapter...\n"));
	while(DxgiAdapter->lpVtbl->EnumOutputs(DxgiAdapter, i, &pOutput) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC* pDesc = &desc;

		hr = pOutput->lpVtbl->GetDesc(pOutput, pDesc);
		if (FAILED(hr))
		{
			_tprintf(_T("Failed to get description\n"));
			return 1;
		}

		_tprintf(_T("Output %d: [%s] [%d]\n"), i, pDesc->DeviceName, pDesc->AttachedToDesktop);

		if(pDesc->AttachedToDesktop)
			dTop = i;

		++i;
	}

	//for now stick to the first one
	dTop = 0;


	_tprintf(_T("\nTrying to get output...\n"));
    hr = DxgiAdapter->lpVtbl->EnumOutputs(DxgiAdapter, dTop, &DxgiOutput);
    DxgiAdapter->lpVtbl->Release(DxgiAdapter);
    DxgiAdapter = NULL;
    if (FAILED(hr))
    {
        _tprintf(_T("Failed to get output\n"));
		return 1;
	}
	_tprintf(_T("Gut!\n"));
	
	//////////////////////////////////////////////

	_tprintf(_T("Trying to get IDXGIOutput1...\n"));
	hr = DxgiOutput->lpVtbl->QueryInterface(DxgiOutput, &IID_IDXGIOutput1, (void**) &DxgiOutput1);
	DxgiOutput->lpVtbl->Release(DxgiOutput);
    DxgiOutput = NULL;
    if (FAILED(hr))
    {
         _tprintf(_T("Failed to get IDXGIOutput1\n"));
		return 1;
    }
	_tprintf(_T("Gut!\n"));
	
	//////////////////////////////////////////////

	_tprintf(_T("Trying to duplicate the output...\n"));
	hr = DxgiOutput1->lpVtbl->DuplicateOutput(DxgiOutput1, (IUnknown*)MeinDevice, &MeinDeskDupl);
	DxgiOutput1->lpVtbl->Release(DxgiOutput1);
    DxgiOutput1 = NULL;
    if (FAILED(hr))
    {
        if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
        {
             _tprintf(_T("There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again.\n"));
			return 1;
        }
		_tprintf(_T("Failed to get duplicate output\n"));
		return 1;
    }
	_tprintf(_T("Gut! Init Complete!\n"));

	
	///////////////////////////////////////////////
	///////////////////////////////////////////////
	///////////////////////////////////////////////

	

	t = 0;
	while(t<5)
	{
		_tprintf(_T("\nTrying to acquire a frame...\n"));
		hr = MeinDeskDupl->lpVtbl->AcquireNextFrame(MeinDeskDupl, 500, &FrameInfo, &DesktopResource);
		_tprintf(_T("hr = %#0X\n"), hr);
		if (hr == DXGI_ERROR_WAIT_TIMEOUT)
		{
			_tprintf(_T("Timeout\n"));
			return 1;
		}

		if (FAILED(hr))
		{
			_tprintf(_T("Failed to acquire next frame\n"));
			return 1;
		}
		_tprintf(_T("Gut!\n"));

		///////////////////////////////////////////////

		
		_tprintf(_T("Trying to QI for ID3D11Texture2D...\n"));
		hr = DesktopResource->lpVtbl->QueryInterface(DesktopResource, &IID_ID3D11Texture2D, (void**) &MeinAcquiredDesktopImage);
		DesktopResource->lpVtbl->Release(DesktopResource);
		DesktopResource = NULL;
		if (FAILED(hr))
		{
			_tprintf(_T("Failed to QI for ID3D11Texture2D from acquired IDXGIResource\n"));
			 return 1;
		}
		_tprintf(_T("Gut!\n"));

		_tprintf(_T("FrameInfo\n"));
		_tprintf(_T("\tAccumulated Frames: %d\n"), FrameInfo.AccumulatedFrames);
		_tprintf(_T("\tCoalesced Rectangles: %d\n"), FrameInfo.RectsCoalesced);
		_tprintf(_T("\tMetadata buffer size: %d\n"), FrameInfo.TotalMetadataBufferSize);


		if(FrameInfo.TotalMetadataBufferSize)
		{

			if (FrameInfo.TotalMetadataBufferSize > MeinMetaDataSize)
			{
				if (MeinMetaDataBuffer)
				{
					free(MeinMetaDataBuffer);
					MeinMetaDataBuffer = NULL;
				}
				MeinMetaDataBuffer = (BYTE*) malloc(FrameInfo.TotalMetadataBufferSize);
				if (!MeinMetaDataBuffer)
				{
					MeinMetaDataSize = 0;
					_tprintf(_T("Failed to allocate memory for metadata\n"));
					return 1;
				}
				MeinMetaDataSize = FrameInfo.TotalMetadataBufferSize;
			}

			BufSize = FrameInfo.TotalMetadataBufferSize;

			// Get move rectangles
			hr = MeinDeskDupl->lpVtbl->GetFrameMoveRects(MeinDeskDupl, BufSize, (DXGI_OUTDUPL_MOVE_RECT*) MeinMetaDataBuffer, &BufSize);
			if (FAILED(hr))
			{
				_tprintf(_T("Failed to get frame move rects\n"));
				return 1;
			}
			_tprintf(_T("Move rects: %d\n"), BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT));

			DirtyRects = MeinMetaDataBuffer + BufSize;
			BufSize = FrameInfo.TotalMetadataBufferSize - BufSize;

			 // Get dirty rectangles
			hr = MeinDeskDupl->lpVtbl->GetFrameDirtyRects(MeinDeskDupl, BufSize, (RECT*) DirtyRects, &BufSize);
			if (FAILED(hr))
			{
				_tprintf(_T("Failed to get frame dirty rects\n"));
				return 1;
			}
			dirty = BufSize / sizeof(RECT);
			_tprintf(_T("Dirty rects: %d\n"), dirty);

			pRect = (RECT*) DirtyRects;
			for(i = 0; i<dirty; ++i)
			{
				_tprintf(_T("\tRect: (%d, %d), (%d, %d)\n"),
					pRect->left,
					pRect->top,
					pRect->right,
					pRect->bottom);

				++pRect;
			}
			

		}


		hr = MeinDeskDupl->lpVtbl->ReleaseFrame(MeinDeskDupl);
		if (FAILED(hr))
		{
			_tprintf(_T("Failed to release frame\n"));
			return 1;
		}

		Sleep(200);
		++t;
	}

	
	return 0;
}

