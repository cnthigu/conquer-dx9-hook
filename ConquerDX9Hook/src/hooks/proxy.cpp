#include <windows.h>
#include <map>
#include <string>

std::map<std::string, FARPROC> g_originalFunctionCache;


HMODULE g_originalChatDllHandle = nullptr;

FARPROC GetOriginalFunctionAddress(const char* functionName) 
{
	if (!g_originalFunctionCache[functionName]) 
	{
		g_originalFunctionCache[functionName] = GetProcAddress(g_originalChatDllHandle, functionName);
	}
	return g_originalFunctionCache[functionName];
}

extern "C" 
{
	typedef void* (*ChaterInfoMgrQueryFunc)();
	typedef int(__cdecl* ChatInfoManagerDestroyFunc)(int, int);

	__declspec(dllexport) void* ChaterInfoMgrQuery() 
	{
		ChaterInfoMgrQueryFunc originalFunc = (ChaterInfoMgrQueryFunc)GetOriginalFunctionAddress("ChaterInfoMgrQuery");
		return originalFunc ? originalFunc() : nullptr;
	}

	__declspec(dllexport) int __cdecl ChatInfoManagerDestroy(int param1, int param2)
	{
		ChatInfoManagerDestroyFunc originalFunc = (ChatInfoManagerDestroyFunc)GetOriginalFunctionAddress("ChatInfoManagerDestroy");
		return originalFunc ? originalFunc(param1, param2) : 0;
	}
}

