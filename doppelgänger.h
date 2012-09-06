#ifndef doppel_h
#define doppel_h

#define CINTERFACE

#include <D3D11.h>
#include <dxgi1_2.h>

// Driver types supported
D3D_DRIVER_TYPE DriverTypes[] =
{
	D3D_DRIVER_TYPE_HARDWARE,
	D3D_DRIVER_TYPE_WARP,
	D3D_DRIVER_TYPE_REFERENCE,
};
UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

// Feature levels supported
D3D_FEATURE_LEVEL FeatureLevels[] =
{
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_1
};
UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

D3D_FEATURE_LEVEL FeatureLevel;

#endif