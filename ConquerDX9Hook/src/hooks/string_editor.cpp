#include <windows.h>
#include <d3d9.h>
#include <string>
#include <vector>
#include <cmath>
#include "MinHook.h"
#include "common.h"

extern GameWindowInfo g_gameWindow;
extern ShowStringExFunc g_originalShowStringExFunction;
extern std::vector<StringConfiguration> g_itemStringConfigurations;
extern std::vector<CapturedStringData> g_capturedStrings;

std::string g_serverName = "WarLord";             
bool g_serverNameEnabled = true;                  
bool g_serverNameUseRainbow = false;             
bool g_serverNameUseCustomColor = false;          
float g_serverNameColorRed = 1.0f;                
float g_serverNameColorGreen = 1.0f;              
float g_serverNameColorBlue = 1.0f;              
float g_serverNameColorAlpha = 1.0f;              


float g_rainbowSpeed = 1.0f;                      
float g_rainbowRed = 0.0f;                        
float g_rainbowGreen = 0.0f;                      
float g_rainbowBlue = 0.0f;                       
float g_rainbowTime = 0.0f;                      
DWORD g_rainbowLastTick = 0;                      

LPVOID g_originalShowStringExAddress = nullptr;
bool g_isShowStringHookInstalled = false;

extern Direct3DRenderStateBackup SaveDirect3DRenderStates(LPDIRECT3DDEVICE9 device);
extern void RestoreDirect3DRenderStates(LPDIRECT3DDEVICE9 device, const Direct3DRenderStateBackup& backup);
extern void SetDirect3DRenderStatesFor2D(LPDIRECT3DDEVICE9 device);
extern void DrawStringBackgroundRectangle(LPDIRECT3DDEVICE9 device, int x1, int y1, int x2, int y2, DWORD color);

void UpdateRainbowColors() 
{
	DWORD currentTick = GetTickCount();
	
	if (g_rainbowLastTick == 0) 
	{
		g_rainbowLastTick = currentTick;
		return;
	}
	
	float deltaTime = (currentTick - g_rainbowLastTick) / 1000.0f;
	g_rainbowLastTick = currentTick;
	
	g_rainbowTime += deltaTime * g_rainbowSpeed;
	

	g_rainbowRed = (sin(g_rainbowTime + 0.0f) + 1.0f) / 2.0f;
	g_rainbowGreen = (sin(g_rainbowTime + 2.0f) + 1.0f) / 2.0f;
	g_rainbowBlue = (sin(g_rainbowTime + 4.0f) + 1.0f) / 2.0f;
}

DWORD GetRainbowColor() 
{
	return D3DCOLOR_RGBA(
		static_cast<int>(g_rainbowRed * 255.0f + 0.5f),      
		static_cast<int>(g_rainbowGreen * 255.0f + 0.5f),
		static_cast<int>(g_rainbowBlue * 255.0f + 0.5f),
		255  
	);
}

const StringConfiguration* FindItemStringConfiguration(const std::string& matchedString) 
{
	for (const auto& config : g_itemStringConfigurations) 
	{
		if (config.searchString == matchedString) 
		{
			return &config;
		}
	}
	return nullptr;
}


bool IsServerNameString(const char* stringText) 
{
	if (!stringText || g_serverName.empty()) return false;
	return strstr(stringText, g_serverName.c_str()) != nullptr;
}

Size2D __cdecl HookedShowStringEx(int positionX, int positionY, unsigned long color,
	const char* stringText, const char* fontName, int fontSize, bool isAntialiased,
	TextRenderStyle style, unsigned long secondColor, Position2D offset) 
{
	if (!g_originalShowStringExFunction) 
	{
		return Size2D{ 0, 0 };
	}
	
	if (!stringText) 
	{
		return g_originalShowStringExFunction(positionX, positionY, color, stringText, fontName, fontSize, isAntialiased, style, secondColor, offset);
	}

	bool isServerName = IsServerNameString(stringText);
	
	StringConfiguration* matchingItemConfig = nullptr;
	for (auto& config : g_itemStringConfigurations) 
	{
		if (config.isEnabled && strstr(stringText, config.searchString.c_str()) != nullptr) {
			matchingItemConfig = &config;
			break;
		}
	}

	bool shouldCapture = false;        
	bool shouldUseRainbow = false;      
	bool shouldUseCustomColor = false;  
	DWORD customColor = color;          
	std::string matchedString = "";     
	

	if (isServerName && g_serverNameEnabled) 
	{
		shouldCapture = true;
		matchedString = "SERVER_NAME";
		
		if (g_serverNameUseRainbow) 
		{
			shouldUseRainbow = true;
		} else if (g_serverNameUseCustomColor) 
		{
			shouldUseCustomColor = true;

			customColor = D3DCOLOR_RGBA(
				static_cast<int>(g_serverNameColorRed * 255.0f + 0.5f),
				static_cast<int>(g_serverNameColorGreen * 255.0f + 0.5f),
				static_cast<int>(g_serverNameColorBlue * 255.0f + 0.5f),
				static_cast<int>(g_serverNameColorAlpha * 255.0f + 0.5f)
			);
		}
	} 

	else if (matchingItemConfig) 
	{
		shouldCapture = true;
		matchedString = matchingItemConfig->searchString;
		
		if (matchingItemConfig->useRainbowColor) 
		{
			shouldUseRainbow = true;
		} else if (matchingItemConfig->useCustomTextColor) 
		{
			shouldUseCustomColor = true;
			customColor = D3DCOLOR_RGBA(
				static_cast<int>(matchingItemConfig->textColorRed * 255.0f + 0.5f),
				static_cast<int>(matchingItemConfig->textColorGreen * 255.0f + 0.5f),
				static_cast<int>(matchingItemConfig->textColorBlue * 255.0f + 0.5f),
				static_cast<int>(matchingItemConfig->textColorAlpha * 255.0f + 0.5f)
			);
		}
	}
	
	if (shouldCapture) 
	{
		CapturedStringData captured;
		captured.text = stringText;
		captured.fontName = fontName ? fontName : "";
		captured.positionX = positionX;
		captured.positionY = positionY;
		
		if (shouldUseRainbow) 
		{
			captured.color = GetRainbowColor();
		} else if (shouldUseCustomColor) 
		{
			captured.color = customColor;
		} else 
		{
			captured.color = color;  
		}
		
		captured.fontSize = fontSize;
		captured.isAntialiased = isAntialiased;
		captured.style = style;
		captured.secondColor = secondColor;
		captured.offset = offset;
		captured.matchedSearchString = matchedString;
		
		if (g_capturedStrings.size() < RenderingConstants::MAX_CAPTURED_STRINGS) 
		{
			g_capturedStrings.push_back(captured);
		}
		
		return Size2D{ 0, 0 };
	}

	return g_originalShowStringExFunction(positionX, positionY, color, stringText, fontName, fontSize, isAntialiased, style, secondColor, offset);
}

void InstallShowStringExHook() 
{
	if (g_isShowStringHookInstalled) 
		return;

	HMODULE graphicModuleHandle = GetModuleHandleA("graphic.dll");

	if (!graphicModuleHandle) 
		return;

	const char* showStringExMangledName = "?ShowStringEx@CMyBitmap@@SA?AUC3_SIZE@@HHKPBD0H_NW4RENDER_TEXT_STYLE@@KUC3_POS@@@Z";

	FARPROC showStringExAddress = GetProcAddress(graphicModuleHandle, showStringExMangledName);
	
	if (showStringExAddress) 
	{
		g_originalShowStringExAddress = (LPVOID)showStringExAddress;
		g_originalShowStringExFunction = (ShowStringExFunc)showStringExAddress;

		if (MH_CreateHook(g_originalShowStringExAddress, (LPVOID)HookedShowStringEx, (LPVOID*)&g_originalShowStringExFunction) == MH_OK) 
		{
			MH_EnableHook(g_originalShowStringExAddress);
			g_isShowStringHookInstalled = true;
		}
	}
}

