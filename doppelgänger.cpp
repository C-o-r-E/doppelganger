// doppelgänger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "doppelgänger.h"

HRESULT hr;
ID3D11Device* MeinDevice;
ID3D11DeviceContext* MeinContext;
IDXGIOutputDuplication* MeinDeskDupl;
ID3D11Texture2D* MeinAcquiredDesktopImage;
BYTE* MeinMetaDataBuffer = nullptr;
UINT MeinMetaDataSize = 0;

int _tmain(int argc, _TCHAR* argv[])
{


	///////////////////////////////////////////////////////
	_tprintf(_T("Hallo, welt!\n"));

	_tprintf(_T("Trying to create a DX11 Device...\n"));
	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
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
	IDXGIDevice* DxgiDevice = nullptr;
	hr = MeinDevice->QueryInterface(__uuidof(IDXGIDevice), (void**) &DxgiDevice);
	if (FAILED(hr))
    {
		_tprintf(_T("Failed to get QI for DXGI Device\n"));
        return 1;
    }
	_tprintf(_T("Gut!\n"));


	//////////////////////////////////////////////////////////

	_tprintf(_T("Trying to get adapter for DXGI Device...\n"));
	IDXGIAdapter* DxgiAdapter = nullptr;
    hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**) &DxgiAdapter);
    DxgiDevice->Release();
    DxgiDevice = nullptr;
    if (FAILED(hr))
    {
        _tprintf(_T("Failed to get parent DXGI Adapter\n"));
		return 1;
    }
	_tprintf(_T("Gut!\n"));

	////////////////////////////////////////////////////////////

	DXGI_OUTPUT_DESC desc;
	memset(&desc, 0, sizeof(desc));

	UINT i = 0;
	UINT dTop;
	IDXGIOutput * pOutput;
	_tprintf(_T("\nLooping through ouputs on DXGI adapter...\n"));
	while(DxgiAdapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC* pDesc = &desc;

		hr = pOutput->GetDesc(pDesc);
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
	dTop = 1;


	_tprintf(_T("\nTrying to get output...\n"));
	IDXGIOutput* DxgiOutput = nullptr;
    hr = DxgiAdapter->EnumOutputs(dTop, &DxgiOutput);
    DxgiAdapter->Release();
    DxgiAdapter = nullptr;
    if (FAILED(hr))
    {
        _tprintf(_T("Failed to get output\n"));
		return 1;
	}
	_tprintf(_T("Gut!\n"));
	//////////////////////////////////////////////

	_tprintf(_T("Trying to get IDXGIOutput1...\n"));
	IDXGIOutput1* DxgiOutput1 = nullptr;
    hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), (void**) &DxgiOutput1);
    DxgiOutput->Release();
    DxgiOutput = nullptr;
    if (FAILED(hr))
    {
         _tprintf(_T("Failed to get IDXGIOutput1\n"));
		return 1;
    }
	_tprintf(_T("Gut!\n"));

	//////////////////////////////////////////////

	_tprintf(_T("Trying to duplicate the output...\n"));
	hr = DxgiOutput1->DuplicateOutput(MeinDevice, &MeinDeskDupl);
    DxgiOutput1->Release();
    DxgiOutput1 = nullptr;
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

	IDXGIResource* DesktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO FrameInfo;

	UINT t = 0;
	while(t<5)
	{
		_tprintf(_T("\nTrying to acquire a frame...\n"));
		HRESULT hr = MeinDeskDupl->AcquireNextFrame(500, &FrameInfo, &DesktopResource);
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

		//guessing this must be null
		MeinAcquiredDesktopImage = nullptr;
		_tprintf(_T("Trying to QI for ID3D11Texture2D...\n"));
		hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**) &MeinAcquiredDesktopImage);
		DesktopResource->Release();
		DesktopResource = nullptr;
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
					MeinMetaDataBuffer = nullptr;
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

			UINT BufSize = FrameInfo.TotalMetadataBufferSize;

			// Get move rectangles
			hr = MeinDeskDupl->GetFrameMoveRects(BufSize, (DXGI_OUTDUPL_MOVE_RECT*) MeinMetaDataBuffer, &BufSize);
			if (FAILED(hr))
			{
				_tprintf(_T("Failed to get frame move rects\n"));
				return 1;
			}
			_tprintf(_T("Move rects: %d\n"), BufSize / sizeof(DXGI_OUTDUPL_MOVE_RECT));

			BYTE* DirtyRects = MeinMetaDataBuffer + BufSize;
			BufSize = FrameInfo.TotalMetadataBufferSize - BufSize;

			 // Get dirty rectangles
			hr = MeinDeskDupl->GetFrameDirtyRects(BufSize, (RECT*) DirtyRects, &BufSize);
			if (FAILED(hr))
			{
				_tprintf(_T("Failed to get frame dirty rects\n"));
				return 1;
			}
			_tprintf(_T("Dirty rects: %d\n"), BufSize / sizeof(RECT));
			

		}


		hr = MeinDeskDupl->ReleaseFrame();
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

