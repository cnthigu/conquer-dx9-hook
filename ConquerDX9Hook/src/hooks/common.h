#pragma once

#include <windows.h>
#include <d3d9.h>
#include <string>
#include <vector>

typedef HRESULT(WINAPI* EndSceneFunc)(LPDIRECT3DDEVICE9);

typedef HRESULT(WINAPI* ResetFunc)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);

typedef BOOL(WINAPI* GetKeyboardStateFunc)(PBYTE);

typedef HRESULT(WINAPI* DrawIndexedPrimitiveFunc)(LPDIRECT3DDEVICE9, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);


struct Size2D {
	int width;
	int height;
};

struct Position2D {
	int x;
	int y;
};

enum TextRenderStyle {
	TEXT_STYLE_DEFAULT = 0,
	TEXT_STYLE_BOLD = 1,
};


typedef Size2D(__cdecl* ShowStringExFunc)(int, int, unsigned long, const char*, const char*, int, bool, TextRenderStyle, unsigned long, Position2D);

struct StringConfiguration {
	std::string searchString;        
	bool isEnabled = true;            
	bool useCustomTextColor = false;  
	bool useRainbowColor = false;     
	float textColorRed = 1.0f;        
	float textColorGreen = 1.0f;      
	float textColorBlue = 1.0f;       
	float textColorAlpha = 1.0f;      
	bool showBackground = true;       
	
	StringConfiguration() {}
	StringConfiguration(const std::string& str) : searchString(str) {}
};

struct CapturedStringData {
	std::string text;                 
	std::string fontName;             
	std::string matchedSearchString;  
	int positionX;                    
	int positionY;                    
	DWORD color;                      
	int fontSize;                     
	bool isAntialiased;               
	TextRenderStyle style;            
	DWORD secondColor;                
	Position2D offset;                
	
	CapturedStringData() : positionX(0), positionY(0), color(0), fontSize(0), 
		isAntialiased(false), style(TEXT_STYLE_DEFAULT), secondColor(0), offset{0, 0} {}
};

struct GameWindowInfo {
	HWND parentWindowHandle;          
	HWND gameWindowHandle;            
	WNDPROC originalWindowProcedure;  
	LPDIRECT3DDEVICE9 direct3DDevice;  
	bool isGuiWindowOpen;             

	GameWindowInfo() : parentWindowHandle(NULL), gameWindowHandle(NULL), originalWindowProcedure(NULL),
		direct3DDevice(nullptr), isGuiWindowOpen(true) {}
};

struct Direct3DRenderStateBackup {
	DWORD flexibleVertexFormat;       
	DWORD lightingState;              
	DWORD zBufferEnable;              
	DWORD zBufferWriteEnable;         
	DWORD alphaBlendEnable;           
	DWORD sourceBlendMode;            
	DWORD destinationBlendMode;       
	DWORD cullMode;                   
};

namespace RenderingConstants {
	constexpr float TEXT_WIDTH_MULTIPLIER = 0.6f;  
	constexpr int BACKGROUND_PADDING_X = 4;        
	constexpr int BACKGROUND_PADDING_Y = 2;        
	constexpr int BACKGROUND_ALPHA = 200;          
	constexpr size_t MAX_CAPTURED_STRINGS = 1000;  
}