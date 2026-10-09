#include "common.h"


GameWindowInfo g_gameWindow;                                    
bool g_isImGuiInitialized = false;                              
EndSceneFunc g_originalEndSceneFunction = nullptr;              
ResetFunc g_originalResetFunction = nullptr;                   
LPVOID g_originalEndSceneAddress = nullptr;                    
LPVOID g_originalResetAddress = nullptr;                       
ShowStringExFunc g_originalShowStringExFunction = nullptr;      
std::vector<StringConfiguration> g_itemStringConfigurations;    
std::vector<CapturedStringData> g_capturedStrings;              

uintptr_t FindMemoryPattern(uintptr_t startAddress, size_t searchLength, const std::vector<int>& pattern) 
{
	const uint8_t* memoryData = reinterpret_cast<const uint8_t*>(startAddress);

	size_t patternLength = pattern.size();

	for (size_t i = 0; i <= searchLength - patternLength; ++i) 
	{
		bool patternMatches = true;
		
		for (size_t j = 0; j < patternLength; ++j) {
			if (pattern[j] != -1 && pattern[j] != memoryData[i + j]) 
			{
				patternMatches = false;
				break;
			}
		}

		if (patternMatches) 
			return startAddress + i;
	}
	return 0;  
}

HWND FindGameWindowHandle() 
{
	HWND parentWindowHandle = NULL;

	auto enumWindowsCallback = [](HWND windowHandle, LPARAM lParam) -> BOOL 
		{
			DWORD processId = 0;
			GetWindowThreadProcessId(windowHandle, &processId);

			char windowTitle[256] = { 0 };
			GetWindowTextA(windowHandle, windowTitle, sizeof(windowTitle));

			char className[256] = { 0 };
			GetClassNameA(windowHandle, className, sizeof(className));

			if (GetCurrentProcessId() == processId &&
				(strstr(windowTitle, "Conquer") != nullptr ||  
					strstr(className, "Afx:00400000:0:000100") != nullptr)) 
			{
				*reinterpret_cast<HWND*>(lParam) = windowHandle;
				return FALSE; 
			}
			return TRUE;  
		};

	EnumWindows(enumWindowsCallback, reinterpret_cast<LPARAM>(&parentWindowHandle));

	if (parentWindowHandle == NULL) 
		return NULL;

	g_gameWindow.parentWindowHandle = parentWindowHandle;


	return FindWindowExA(parentWindowHandle, NULL, "#32770", NULL);
}

Direct3DRenderStateBackup SaveDirect3DRenderStates(LPDIRECT3DDEVICE9 device) 
{
	Direct3DRenderStateBackup backup;
	
	// Salva cada estado importante
	device->GetFVF(&backup.flexibleVertexFormat);
	device->GetRenderState(D3DRS_LIGHTING, &backup.lightingState);
	device->GetRenderState(D3DRS_ZENABLE, &backup.zBufferEnable);
	device->GetRenderState(D3DRS_ZWRITEENABLE, &backup.zBufferWriteEnable);
	device->GetRenderState(D3DRS_ALPHABLENDENABLE, &backup.alphaBlendEnable);
	device->GetRenderState(D3DRS_SRCBLEND, &backup.sourceBlendMode);
	device->GetRenderState(D3DRS_DESTBLEND, &backup.destinationBlendMode);
	device->GetRenderState(D3DRS_CULLMODE, &backup.cullMode);
	
	return backup;
}

void RestoreDirect3DRenderStates(LPDIRECT3DDEVICE9 device, const Direct3DRenderStateBackup& backup) 
{
	// Restaura cada estado que foi salvo
	device->SetFVF(backup.flexibleVertexFormat);
	device->SetRenderState(D3DRS_LIGHTING, backup.lightingState);
	device->SetRenderState(D3DRS_ZENABLE, backup.zBufferEnable);
	device->SetRenderState(D3DRS_ZWRITEENABLE, backup.zBufferWriteEnable);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, backup.alphaBlendEnable);
	device->SetRenderState(D3DRS_SRCBLEND, backup.sourceBlendMode);
	device->SetRenderState(D3DRS_DESTBLEND, backup.destinationBlendMode);
	device->SetRenderState(D3DRS_CULLMODE, backup.cullMode);
}

void SetDirect3DRenderStatesFor2D(LPDIRECT3DDEVICE9 device) 
{
	device->SetRenderState(D3DRS_LIGHTING, FALSE);	
	device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->SetTexture(0, NULL);
	device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
}

void DrawStringBackgroundRectangle(LPDIRECT3DDEVICE9 device, int x1, int y1, int x2, int y2, DWORD color) 
{
	if (x2 <= x1 || y2 <= y1) 
		return;

	struct Vertex 
	{
		float x, y, z, rhw;
		DWORD color;
	};

	Vertex vertices[4] = 
	{
		{ (float)x1, (float)y1, 0.0f, 1.0f, color },  
		{ (float)x2, (float)y1, 0.0f, 1.0f, color },  
		{ (float)x1, (float)y2, 0.0f, 1.0f, color },  
		{ (float)x2, (float)y2, 0.0f, 1.0f, color }   
	};

	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
}



