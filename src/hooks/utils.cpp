#include "utils.h"
#include "directx_hooks.h"

uintptr_t FindMemoryPattern ( uintptr_t start_address, size_t search_length, const std::vector<int>& pattern )
{
    const uint8_t* memory_data = reinterpret_cast<const uint8_t*> ( start_address );
    size_t pattern_length = pattern.size ();

    for ( size_t i = 0; i <= search_length - pattern_length; ++i )
    {
        bool pattern_matches = true;

        for ( size_t j = 0; j < pattern_length; ++j )
        {
            if ( pattern[j] != -1 && pattern[j] != memory_data[i + j] )
            {
                pattern_matches = false;
                break;
            }
        }

        if ( pattern_matches )
        {
            return start_address + i;
        }
    }

    return 0;
}

HWND FindGameWindowHandle ()
{
    HWND parent_window_handle = NULL;

    auto enum_windows_callback = [] ( HWND window_handle, LPARAM l_param ) -> BOOL
    {
        DWORD process_id = 0;
        GetWindowThreadProcessId ( window_handle, &process_id );

        char window_title[256] = { 0 };
        GetWindowTextA ( window_handle, window_title, sizeof ( window_title ) );

        char class_name[256] = { 0 };
        GetClassNameA ( window_handle, class_name, sizeof ( class_name ) );

        if ( GetCurrentProcessId () == process_id && ( strstr ( window_title, "Conquer" ) != nullptr ||
                                                       strstr ( class_name, "Afx:00400000:0:000100" ) != nullptr ) )
        {
            *reinterpret_cast<HWND*> ( l_param ) = window_handle;
            return FALSE;
        }

        return TRUE;
    };

    EnumWindows ( enum_windows_callback, reinterpret_cast<LPARAM> ( &parent_window_handle ) );

    if ( parent_window_handle == NULL )
    {
        return NULL;
    }

    game_window.parent_window_handle = parent_window_handle;
    return FindWindowExA ( parent_window_handle, NULL, "#32770", NULL );
}

Direct3DRenderStateBackup SaveDirect3DRenderStates ( LPDIRECT3DDEVICE9 device )
{
    Direct3DRenderStateBackup backup;
    device->GetFVF ( &backup.flexible_vertex_format );
    device->GetRenderState ( D3DRS_LIGHTING, &backup.lighting_state );
    device->GetRenderState ( D3DRS_ZENABLE, &backup.z_buffer_enable );
    device->GetRenderState ( D3DRS_ZWRITEENABLE, &backup.z_buffer_write_enable );
    device->GetRenderState ( D3DRS_ALPHABLENDENABLE, &backup.alpha_blend_enable );
    device->GetRenderState ( D3DRS_SRCBLEND, &backup.source_blend_mode );
    device->GetRenderState ( D3DRS_DESTBLEND, &backup.destination_blend_mode );
    device->GetRenderState ( D3DRS_CULLMODE, &backup.cull_mode );
    return backup;
}

void RestoreDirect3DRenderStates ( LPDIRECT3DDEVICE9 device, const Direct3DRenderStateBackup& backup )
{
    device->SetFVF ( backup.flexible_vertex_format );
    device->SetRenderState ( D3DRS_LIGHTING, backup.lighting_state );
    device->SetRenderState ( D3DRS_ZENABLE, backup.z_buffer_enable );
    device->SetRenderState ( D3DRS_ZWRITEENABLE, backup.z_buffer_write_enable );
    device->SetRenderState ( D3DRS_ALPHABLENDENABLE, backup.alpha_blend_enable );
    device->SetRenderState ( D3DRS_SRCBLEND, backup.source_blend_mode );
    device->SetRenderState ( D3DRS_DESTBLEND, backup.destination_blend_mode );
    device->SetRenderState ( D3DRS_CULLMODE, backup.cull_mode );
}

void SetDirect3DRenderStatesFor2D ( LPDIRECT3DDEVICE9 device )
{
    device->SetRenderState ( D3DRS_LIGHTING, FALSE );
    device->SetRenderState ( D3DRS_ZENABLE, D3DZB_FALSE );
    device->SetRenderState ( D3DRS_ZWRITEENABLE, FALSE );
    device->SetRenderState ( D3DRS_ALPHABLENDENABLE, TRUE );
    device->SetRenderState ( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    device->SetRenderState ( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    device->SetRenderState ( D3DRS_CULLMODE, D3DCULL_NONE );
    device->SetFVF ( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
    device->SetTexture ( 0, NULL );
    device->SetTextureStageState ( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    device->SetTextureStageState ( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
}

void DrawStringBackgroundRectangle ( LPDIRECT3DDEVICE9 device, int x1, int y1, int x2, int y2, DWORD color )
{
    if ( x2 <= x1 || y2 <= y1 )
    {
        return;
    }

    struct Vertex
    {
        float x, y, z, rhw;
        DWORD color;
    };

    Vertex vertices[4] = { { (float)x1, (float)y1, 0.0f, 1.0f, color },
                           { (float)x2, (float)y1, 0.0f, 1.0f, color },
                           { (float)x1, (float)y2, 0.0f, 1.0f, color },
                           { (float)x2, (float)y2, 0.0f, 1.0f, color } };

    device->DrawPrimitiveUP ( D3DPT_TRIANGLESTRIP, 2, vertices, sizeof ( Vertex ) );
}
