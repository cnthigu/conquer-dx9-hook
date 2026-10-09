#include <windows.h>
#include "hooks/common.h"
#include "MinHook.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "libMinHook.x86.lib")

extern HMODULE g_originalChatDllHandle;
extern std::vector<StringConfiguration> g_itemStringConfigurations;
extern GameWindowInfo g_gameWindow;
extern EndSceneFunc g_originalEndSceneFunction;
extern ResetFunc g_originalResetFunction;
extern LPVOID g_originalEndSceneAddress;
extern LPVOID g_originalResetAddress;
extern void InstallWindowProcedureHook();
extern void InstallGetKeyboardStateHook();
extern void InstallDrawIndexedPrimitiveHook();
extern void InstallShowStringExHook();
extern uintptr_t FindMemoryPattern(uintptr_t startAddress, size_t searchLength, const std::vector<int>& pattern);
extern HRESULT WINAPI HookedEndScene(LPDIRECT3DDEVICE9 device);
extern HRESULT WINAPI HookedReset(LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* presentationParameters);


void HookInitializationThread() 
{
	HMODULE direct3D9ModuleHandle = nullptr;

	while (!(direct3D9ModuleHandle = GetModuleHandleA("d3d9.dll"))) 
	{
		Sleep(100); 
	}

	uintptr_t direct3D9ModuleBaseAddress = reinterpret_cast<uintptr_t>(direct3D9ModuleHandle);

	std::vector<int> vmtPattern = { 0xC7, 0x06, -1, -1, -1, -1, 0x89, 0x86, -1, -1, -1, -1, 0x89, 0x86 };
	
	size_t moduleSize = 0x100000;


	if (direct3D9ModuleHandle) 
	{
		PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)direct3D9ModuleHandle;
		if (dosHeader && dosHeader->e_magic == IMAGE_DOS_SIGNATURE) 
		{
			PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((uintptr_t)direct3D9ModuleHandle + dosHeader->e_lfanew);
			if (ntHeaders && ntHeaders->Signature == IMAGE_NT_SIGNATURE) 
			{
				moduleSize = ntHeaders->OptionalHeader.SizeOfImage;
			}
		}
	}
	
	uintptr_t vmtBaseAddress = FindMemoryPattern(direct3D9ModuleBaseAddress, moduleSize, vmtPattern);

	if (!vmtBaseAddress) 
	{
		return;
	}
	
	uintptr_t* virtualMethodTable = *reinterpret_cast<uintptr_t**>(vmtBaseAddress + 2);
	EndSceneFunc originalEndSceneFunc = reinterpret_cast<EndSceneFunc>(virtualMethodTable[42]);
	ResetFunc originalResetFunc = reinterpret_cast<ResetFunc>(virtualMethodTable[16]);


	g_originalEndSceneAddress = (LPVOID)originalEndSceneFunc;
	g_originalResetAddress = (LPVOID)originalResetFunc;


	MH_Initialize();
	MH_CreateHook(g_originalEndSceneAddress, (LPVOID)HookedEndScene, (LPVOID*)&g_originalEndSceneFunction);
	MH_CreateHook(g_originalResetAddress, (LPVOID)HookedReset, (LPVOID*)&g_originalResetFunction);
	
	InstallWindowProcedureHook();
	
	MH_EnableHook(g_originalEndSceneAddress);
	MH_EnableHook(g_originalResetAddress);
	
	InstallGetKeyboardStateHook();
	
	while (!g_gameWindow.direct3DDevice) 
	{
		Sleep(100);
	}
	InstallDrawIndexedPrimitiveHook();

	while (!GetModuleHandleA("graphic.dll")) 
	{
		Sleep(100);
	}
	InstallShowStringExHook();

	while (true) 
	{
		Sleep(16);  

		if (GetAsyncKeyState(VK_INSERT) & 1) 
		{
			g_gameWindow.isGuiWindowOpen = !g_gameWindow.isGuiWindowOpen;
			Sleep(200); 
		}
	}
}


BOOL APIENTRY DllMain(HMODULE moduleHandle, DWORD reason, LPVOID reserved) 
{
	switch (reason) 
	{
	case DLL_PROCESS_ATTACH:
	
		DisableThreadLibraryCalls(moduleHandle);
		
		g_originalChatDllHandle = LoadLibraryA("OChat.dll");

		g_itemStringConfigurations.push_back(StringConfiguration("WhiteDye")); /// itens blackground/strings edit

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)HookInitializationThread, NULL, 0, NULL);
		break;
		
	case DLL_PROCESS_DETACH:
		
		if (g_originalChatDllHandle) 
		{
			FreeLibrary(g_originalChatDllHandle);
		}
		break;
	}
	return TRUE;
}
