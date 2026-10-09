#include "hooks/hooks.h"
#include "hooks/proxy.h"
#include "hooks/string_editor.h"

BOOL APIENTRY DllMain ( HMODULE module_handle, DWORD reason, LPVOID reserved )
{
    switch ( reason )
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls ( module_handle );

        InitializeChatProxy ();

        item_string_configurations.push_back ( StringConfiguration ( "WhiteDye" ) );

        CreateThread ( NULL, 0, InitializeHooks, NULL, 0, NULL );

        break;

    case DLL_PROCESS_DETACH:
        ShutdownChatProxy ();
        break;
    }

    return TRUE;
}
