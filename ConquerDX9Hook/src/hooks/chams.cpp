#include <windows.h>
#include <d3d9.h>
#include "MinHook.h"
#include "common.h"

extern GameWindowInfo g_gameWindow;


bool g_isWireframeEnabled = false;  

DrawIndexedPrimitiveFunc g_originalDrawIndexedPrimitiveFunction = nullptr;
LPVOID g_originalDrawIndexedPrimitiveAddress = nullptr;
bool g_isDrawIndexedPrimitiveHookInstalled = false;

HRESULT WINAPI HookedDrawIndexedPrimitive(LPDIRECT3DDEVICE9 device, D3DPRIMITIVETYPE primitiveType, 
	INT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT startIndex, UINT primitiveCount) 
{

	if (g_isWireframeEnabled && device) 
	{
		DWORD originalFillMode;
		device->GetRenderState(D3DRS_FILLMODE, &originalFillMode);
		
		device->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		
		HRESULT result = g_originalDrawIndexedPrimitiveFunction(device, primitiveType, baseVertexIndex, 
			minVertexIndex, numVertices, startIndex, primitiveCount);
		

		device->SetRenderState(D3DRS_FILLMODE, originalFillMode);
		return result;
	}
	
	return g_originalDrawIndexedPrimitiveFunction(device, primitiveType, baseVertexIndex, 
		minVertexIndex, numVertices, startIndex, primitiveCount);
}

void InstallDrawIndexedPrimitiveHook() 
{
	if (g_isDrawIndexedPrimitiveHookInstalled) 
		return;
	
	if (!g_gameWindow.direct3DDevice) 
		return;
	
	uintptr_t* virtualMethodTable = *reinterpret_cast<uintptr_t**>(g_gameWindow.direct3DDevice);
	if (!virtualMethodTable) 
		return;
	
	DrawIndexedPrimitiveFunc originalDrawIndexedPrimitiveFunc = reinterpret_cast<DrawIndexedPrimitiveFunc>(virtualMethodTable[82]);
	
	if (!originalDrawIndexedPrimitiveFunc) 
		return;
	
	g_originalDrawIndexedPrimitiveAddress = (LPVOID)originalDrawIndexedPrimitiveFunc;
	g_originalDrawIndexedPrimitiveFunction = originalDrawIndexedPrimitiveFunc;
	
	if (MH_CreateHook(g_originalDrawIndexedPrimitiveAddress, (LPVOID)HookedDrawIndexedPrimitive, 
		(LPVOID*)&g_originalDrawIndexedPrimitiveFunction) == MH_OK) 
	{
		MH_EnableHook(g_originalDrawIndexedPrimitiveAddress);
		g_isDrawIndexedPrimitiveHookInstalled = true;
	}
}

