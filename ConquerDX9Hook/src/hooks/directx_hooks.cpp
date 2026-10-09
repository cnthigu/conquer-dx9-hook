#include <windows.h>
#include <d3d9.h>
#include <vector>
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "MinHook.h"
#include "common.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern GameWindowInfo g_gameWindow;
extern bool g_isImGuiInitialized;
extern EndSceneFunc g_originalEndSceneFunction;
extern ResetFunc g_originalResetFunction;
extern LPVOID g_originalEndSceneAddress;
extern ShowStringExFunc g_originalShowStringExFunction;
extern std::vector<CapturedStringData> g_capturedStrings;
extern void RenderImGuiInterface();
extern void UpdateRainbowColors();
extern HWND FindGameWindowHandle();
extern Direct3DRenderStateBackup SaveDirect3DRenderStates(LPDIRECT3DDEVICE9 device);
extern void RestoreDirect3DRenderStates(LPDIRECT3DDEVICE9 device, const Direct3DRenderStateBackup& backup);
extern void SetDirect3DRenderStatesFor2D(LPDIRECT3DDEVICE9 device);
extern void DrawStringBackgroundRectangle(LPDIRECT3DDEVICE9 device, int x1, int y1, int x2, int y2, DWORD color);
extern const StringConfiguration* FindItemStringConfiguration(const std::string& matchedString);
extern LPVOID g_originalShowStringExAddress;

HRESULT WINAPI HookedEndScene(LPDIRECT3DDEVICE9 device) 
{

	if (!g_gameWindow.direct3DDevice) 
	{
		g_gameWindow.direct3DDevice = device;
	}

	if (!g_isImGuiInitialized) 
	{
		if (!g_gameWindow.gameWindowHandle) 
		{
			g_gameWindow.gameWindowHandle = FindGameWindowHandle();
		}
		
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;  
		
		ImGui_ImplWin32_Init(g_gameWindow.parentWindowHandle);
		ImGui_ImplDX9_Init(g_gameWindow.direct3DDevice);
		g_isImGuiInitialized = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	RenderImGuiInterface();  
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	
	UpdateRainbowColors();
	
	if (g_originalShowStringExFunction && !g_capturedStrings.empty() && device) 
	{
		DWORD backgroundColor = D3DCOLOR_RGBA(0, 0, 0, RenderingConstants::BACKGROUND_ALPHA);
		
		Direct3DRenderStateBackup renderStateBackup = SaveDirect3DRenderStates(device);
		SetDirect3DRenderStatesFor2D(device);
		
		for (const auto& captured : g_capturedStrings) 
		{
			bool shouldShowBackground = false;
			
			if (captured.matchedSearchString != "SERVER_NAME") 
			{
				const StringConfiguration* config = FindItemStringConfiguration(captured.matchedSearchString);
				shouldShowBackground = config ? config->showBackground : false;
			}
			
			if (shouldShowBackground) 
			{
				int textWidth = static_cast<int>(captured.text.length() * captured.fontSize * RenderingConstants::TEXT_WIDTH_MULTIPLIER);
				int textHeight = captured.fontSize;
				
				int backgroundX1 = captured.positionX - RenderingConstants::BACKGROUND_PADDING_X;
				int backgroundY1 = captured.positionY - RenderingConstants::BACKGROUND_PADDING_Y;
				int backgroundX2 = captured.positionX + textWidth + RenderingConstants::BACKGROUND_PADDING_X;
				int backgroundY2 = captured.positionY + textHeight + RenderingConstants::BACKGROUND_PADDING_Y;
				
				DrawStringBackgroundRectangle(device, backgroundX1, backgroundY1, backgroundX2, backgroundY2, backgroundColor);
			}
		}
		
		for (const auto& captured : g_capturedStrings) 
		{
			const char* fontToUse = captured.fontName.empty() ? NULL : captured.fontName.c_str();
			
			g_originalShowStringExFunction(captured.positionX, captured.positionY, captured.color,
				captured.text.c_str(), fontToUse,
				captured.fontSize, captured.isAntialiased,
				captured.style, captured.secondColor, captured.offset);
		}
		
		RestoreDirect3DRenderStates(device, renderStateBackup);
		
		g_capturedStrings.clear();
	}

	return g_originalEndSceneFunction(device);
}


HRESULT WINAPI HookedReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* presentationParameters) 
{

	ImGui_ImplDX9_InvalidateDeviceObjects();

	MH_DisableHook(g_originalEndSceneAddress);
	g_isImGuiInitialized = false;

	HRESULT result = g_originalResetFunction(device, presentationParameters);


	if (SUCCEEDED(result)) 
	{
		ImGui_ImplDX9_CreateDeviceObjects();

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		MH_EnableHook(g_originalEndSceneAddress);
	}

	return result;
}

LRESULT CALLBACK HookedWindowProcedure(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam) 
{

	if (g_isImGuiInitialized && g_gameWindow.isGuiWindowOpen) 
	{
		ImGuiIO& io = ImGui::GetIO();
		
		ImGui_ImplWin32_WndProcHandler(windowHandle, message, wParam, lParam);

		if (io.WantCaptureMouse || io.WantCaptureKeyboard) 
		{
			switch (message) {
			case WM_LBUTTONDOWN:      
			case WM_LBUTTONUP:        
			case WM_LBUTTONDBLCLK:    
			case WM_RBUTTONDOWN:      
			case WM_RBUTTONUP:       
			case WM_RBUTTONDBLCLK:    
			case WM_MBUTTONDOWN:      
			case WM_MBUTTONUP:        
			case WM_MBUTTONDBLCLK:    
			case WM_MOUSEWHEEL:       
			case WM_MOUSEHWHEEL:      
			case WM_MOUSEMOVE:        
			case WM_KEYDOWN:          
			case WM_KEYUP:            
			case WM_SYSKEYDOWN:       
			case WM_SYSKEYUP:         
			case WM_CHAR:             
			case WM_IME_CHAR:         
			case WM_IME_COMPOSITION:  
				return 0;  
			}
		}
	}
	
	return CallWindowProcA(g_gameWindow.originalWindowProcedure, windowHandle, message, wParam, lParam);
}

void InstallWindowProcedureHook() 
{
	while (!g_gameWindow.gameWindowHandle) 
	{
		g_gameWindow.gameWindowHandle = FindGameWindowHandle();
		
		if (g_gameWindow.gameWindowHandle && !g_gameWindow.originalWindowProcedure) 
		{
			g_gameWindow.originalWindowProcedure = (WNDPROC)SetWindowLongA(g_gameWindow.gameWindowHandle, GWLP_WNDPROC, (LONG_PTR)HookedWindowProcedure);
		}
		Sleep(100);  
	}
}

