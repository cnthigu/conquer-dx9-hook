#include "always_jump.h"
#include "common.h"

#include <MinHook.h>

bool is_always_jump_enabled = false;

static GetKeyboardStateFunc original_get_keyboard_state_function = nullptr;
static LPVOID original_get_keyboard_state_address = nullptr;
static bool is_get_keyboard_state_hook_installed = false;

static BOOL WINAPI HookedGetKeyboardState ( PBYTE key_state_array )
{
    BOOL result = original_get_keyboard_state_function ( key_state_array );

    if ( is_always_jump_enabled && key_state_array )
    {
        key_state_array[VK_CONTROL] |= 0x80;
    }

    return result;
}

void InstallGetKeyboardStateHook ()
{
    if ( is_get_keyboard_state_hook_installed )
    {
        return;
    }

    HMODULE user32_module_handle = GetModuleHandleA ( "user32.dll" );

    if ( !user32_module_handle )
    {
        user32_module_handle = LoadLibraryA ( "user32.dll" );

        if ( !user32_module_handle )
        {
            return;
        }
    }

    FARPROC get_keyboard_state_address = GetProcAddress ( user32_module_handle, "GetKeyboardState" );

    if ( !get_keyboard_state_address )
    {
        return;
    }

    original_get_keyboard_state_address = (LPVOID)get_keyboard_state_address;
    original_get_keyboard_state_function = (GetKeyboardStateFunc)get_keyboard_state_address;

    if ( MH_CreateHook ( original_get_keyboard_state_address, (LPVOID)HookedGetKeyboardState,
                         (LPVOID*)&original_get_keyboard_state_function ) == MH_OK )
    {
        MH_EnableHook ( original_get_keyboard_state_address );
        is_get_keyboard_state_hook_installed = true;
    }
}
