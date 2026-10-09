#include <windows.h>
#include "MinHook.h"
#include "common.h"

extern GameWindowInfo g_gameWindow;

bool g_isAlwaysJumpEnabled = false;  

typedef BOOL(WINAPI* GetKeyboardStateFunc)(PBYTE);
GetKeyboardStateFunc g_originalGetKeyboardStateFunction = nullptr;
LPVOID g_originalGetKeyboardStateAddress = nullptr;
bool g_isGetKeyboardStateHookInstalled = false;

BOOL WINAPI HookedGetKeyboardState(PBYTE keyStateArray) 
{
	BOOL result = g_originalGetKeyboardStateFunction(keyStateArray);
	
	if (g_isAlwaysJumpEnabled && keyStateArray) 
	{
		keyStateArray[VK_CONTROL] |= 0x80;
	}
	return result;
}


void InstallGetKeyboardStateHook() 
{
	if (g_isGetKeyboardStateHookInstalled) 
		return;

	HMODULE user32ModuleHandle = GetModuleHandleA("user32.dll");

	if (!user32ModuleHandle) 
	{
		user32ModuleHandle = LoadLibraryA("user32.dll");

		if (!user32ModuleHandle) 
			return;  
	}

	FARPROC getKeyboardStateAddress = GetProcAddress(user32ModuleHandle, "GetKeyboardState");

	if (!getKeyboardStateAddress) 
		return;  

	g_originalGetKeyboardStateAddress = (LPVOID)getKeyboardStateAddress;
	g_originalGetKeyboardStateFunction = (GetKeyboardStateFunc)getKeyboardStateAddress;


	if (MH_CreateHook(g_originalGetKeyboardStateAddress, (LPVOID)HookedGetKeyboardState, (LPVOID*)&g_originalGetKeyboardStateFunction) == MH_OK) 
	{
		MH_EnableHook(g_originalGetKeyboardStateAddress);
		g_isGetKeyboardStateHookInstalled = true;
	}
}

