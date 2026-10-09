#include "proxy.h"

#include <windows.h>

static HMODULE original_chat_dll_handle = nullptr;

void InitializeChatProxy ()
{
    original_chat_dll_handle = LoadLibraryA ( "OChat.dll" );
}

void ShutdownChatProxy ()
{
    if ( original_chat_dll_handle )
    {
        FreeLibrary ( original_chat_dll_handle );
    }
}

extern "C"
{
    __declspec ( dllexport ) void* ChaterInfoMgrQuery ()
    {
        typedef void* ( *ChaterInfoMgrQueryFunc ) ();
        static ChaterInfoMgrQueryFunc original_function = nullptr;

        if ( !original_function )
        {
            original_function =
                (ChaterInfoMgrQueryFunc)GetProcAddress ( original_chat_dll_handle, "ChaterInfoMgrQuery" );
        }

        return original_function ? original_function () : nullptr;
    }

    __declspec ( dllexport ) int __cdecl ChatInfoManagerDestroy ( int param1, int param2 )
    {
        typedef int ( __cdecl * ChatInfoManagerDestroyFunc ) ( int, int );
        static ChatInfoManagerDestroyFunc original_function = nullptr;

        if ( !original_function )
        {
            original_function =
                (ChatInfoManagerDestroyFunc)GetProcAddress ( original_chat_dll_handle, "ChatInfoManagerDestroy" );
        }

        return original_function ? original_function ( param1, param2 ) : 0;
    }
}
