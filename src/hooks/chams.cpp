#include "chams.h"
#include "directx_hooks.h"

#include <MinHook.h>

bool is_wireframe_enabled = false;

static DrawIndexedPrimitiveFunc original_draw_indexed_primitive_function = nullptr;
static LPVOID original_draw_indexed_primitive_address = nullptr;
static bool is_draw_indexed_primitive_hook_installed = false;

static HRESULT WINAPI HookedDrawIndexedPrimitive ( LPDIRECT3DDEVICE9 device, D3DPRIMITIVETYPE primitive_type,
                                                   INT base_vertex_index, UINT min_vertex_index, UINT num_vertices,
                                                   UINT start_index, UINT primitive_count )
{
    if ( is_wireframe_enabled && device )
    {
        DWORD original_fill_mode;
        device->GetRenderState ( D3DRS_FILLMODE, &original_fill_mode );
        device->SetRenderState ( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

        HRESULT result = original_draw_indexed_primitive_function (
            device, primitive_type, base_vertex_index, min_vertex_index, num_vertices, start_index, primitive_count );

        device->SetRenderState ( D3DRS_FILLMODE, original_fill_mode );
        return result;
    }

    return original_draw_indexed_primitive_function ( device, primitive_type, base_vertex_index, min_vertex_index,
                                                      num_vertices, start_index, primitive_count );
}

void InstallDrawIndexedPrimitiveHook ()
{
    if ( is_draw_indexed_primitive_hook_installed )
    {
        return;
    }

    if ( !game_window.direct3d_device )
    {
        return;
    }

    uintptr_t* virtual_method_table = *reinterpret_cast<uintptr_t**> ( game_window.direct3d_device );

    if ( !virtual_method_table )
    {
        return;
    }

    DrawIndexedPrimitiveFunc original_draw_indexed_primitive_func =
        reinterpret_cast<DrawIndexedPrimitiveFunc> ( virtual_method_table[82] );

    if ( !original_draw_indexed_primitive_func )
    {
        return;
    }

    original_draw_indexed_primitive_address = (LPVOID)original_draw_indexed_primitive_func;
    original_draw_indexed_primitive_function = original_draw_indexed_primitive_func;

    if ( MH_CreateHook ( original_draw_indexed_primitive_address, (LPVOID)HookedDrawIndexedPrimitive,
                         (LPVOID*)&original_draw_indexed_primitive_function ) == MH_OK )
    {
        MH_EnableHook ( original_draw_indexed_primitive_address );
        is_draw_indexed_primitive_hook_installed = true;
    }
}
