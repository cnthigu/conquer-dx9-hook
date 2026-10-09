#include "hooks.h"
#include "always_jump.h"
#include "chams.h"
#include "directx_hooks.h"
#include "string_editor.h"
#include "utils.h"

#include <MinHook.h>

DWORD WINAPI InitializeHooks ( LPVOID parameter )
{
    HMODULE direct3d9_module_handle = nullptr;

    while ( !( direct3d9_module_handle = GetModuleHandleA ( "d3d9.dll" ) ) )
    {
        Sleep ( 100 );
    }

    uintptr_t direct3d9_module_base_address = reinterpret_cast<uintptr_t> ( direct3d9_module_handle );
    std::vector<int> vmt_pattern = { 0xC7, 0x06, -1, -1, -1, -1, 0x89, 0x86, -1, -1, -1, -1, 0x89, 0x86 };
    size_t module_size = 0x100000;

    if ( direct3d9_module_handle )
    {
        PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)direct3d9_module_handle;

        if ( dos_header && dos_header->e_magic == IMAGE_DOS_SIGNATURE )
        {
            PIMAGE_NT_HEADERS nt_headers =
                (PIMAGE_NT_HEADERS)( (uintptr_t)direct3d9_module_handle + dos_header->e_lfanew );

            if ( nt_headers && nt_headers->Signature == IMAGE_NT_SIGNATURE )
            {
                module_size = nt_headers->OptionalHeader.SizeOfImage;
            }
        }
    }

    uintptr_t vmt_base_address = FindMemoryPattern ( direct3d9_module_base_address, module_size, vmt_pattern );

    if ( !vmt_base_address )
    {
        return 0;
    }

    uintptr_t* virtual_method_table = *reinterpret_cast<uintptr_t**> ( vmt_base_address + 2 );
    EndSceneFunc original_end_scene_func = reinterpret_cast<EndSceneFunc> ( virtual_method_table[42] );
    ResetFunc original_reset_func = reinterpret_cast<ResetFunc> ( virtual_method_table[16] );

    original_end_scene_address = (LPVOID)original_end_scene_func;
    original_reset_address = (LPVOID)original_reset_func;

    MH_Initialize ();
    MH_CreateHook ( original_end_scene_address, (LPVOID)HookedEndScene, (LPVOID*)&original_end_scene_function );
    MH_CreateHook ( original_reset_address, (LPVOID)HookedReset, (LPVOID*)&original_reset_function );

    InstallWindowProcedureHook ();

    MH_EnableHook ( original_end_scene_address );
    MH_EnableHook ( original_reset_address );

    InstallGetKeyboardStateHook ();

    while ( !game_window.direct3d_device )
    {
        Sleep ( 100 );
    }

    InstallDrawIndexedPrimitiveHook ();

    while ( !GetModuleHandleA ( "graphic.dll" ) )
    {
        Sleep ( 100 );
    }

    InstallShowStringExHook ();

    while ( true )
    {
        Sleep ( 16 );

        if ( GetAsyncKeyState ( VK_INSERT ) & 1 )
        {
            game_window.is_gui_window_open = !game_window.is_gui_window_open;
            Sleep ( 200 );
        }
    }
}
