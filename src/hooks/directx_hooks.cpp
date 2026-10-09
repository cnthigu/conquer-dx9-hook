#include "directx_hooks.h"
#include "imgui_interface.h"
#include "string_editor.h"
#include "utils.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "MinHook.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler ( HWND window_handle, UINT message, WPARAM w_param,
                                                               LPARAM l_param );

GameWindowInfo game_window;
EndSceneFunc original_end_scene_function = nullptr;
ResetFunc original_reset_function = nullptr;
LPVOID original_end_scene_address = nullptr;
LPVOID original_reset_address = nullptr;

static bool is_imgui_initialized = false;

HRESULT WINAPI HookedEndScene ( LPDIRECT3DDEVICE9 device )
{
    if ( !game_window.direct3d_device )
    {
        game_window.direct3d_device = device;
    }

    if ( !is_imgui_initialized )
    {
        if ( !game_window.game_window_handle )
        {
            game_window.game_window_handle = FindGameWindowHandle ();
        }

        ImGui::CreateContext ();
        ImGuiIO& io = ImGui::GetIO ();
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

        ImGui_ImplWin32_Init ( game_window.parent_window_handle );
        ImGui_ImplDX9_Init ( game_window.direct3d_device );
        is_imgui_initialized = true;
    }

    ImGui_ImplDX9_NewFrame ();
    ImGui_ImplWin32_NewFrame ();
    ImGui::NewFrame ();
    RenderImGuiInterface ();
    ImGui::EndFrame ();
    ImGui::Render ();
    ImGui_ImplDX9_RenderDrawData ( ImGui::GetDrawData () );

    UpdateRainbowColors ();

    if ( original_show_string_ex_function && !captured_strings.empty () && device )
    {
        DWORD background_color = D3DCOLOR_RGBA ( 0, 0, 0, background_alpha );

        Direct3DRenderStateBackup render_state_backup = SaveDirect3DRenderStates ( device );
        SetDirect3DRenderStatesFor2D ( device );

        for ( const auto& captured : captured_strings )
        {
            bool should_show_background = false;

            if ( captured.matched_search_string != "SERVER_NAME" )
            {
                const StringConfiguration* config = FindItemStringConfiguration ( captured.matched_search_string );
                should_show_background = config ? config->show_background : false;
            }

            if ( should_show_background )
            {
                int text_width =
                    static_cast<int> ( captured.text.length () * captured.font_size * text_width_multiplier );
                int text_height = captured.font_size;

                int background_x1 = captured.position_x - background_padding_x;
                int background_y1 = captured.position_y - background_padding_y;
                int background_x2 = captured.position_x + text_width + background_padding_x;
                int background_y2 = captured.position_y + text_height + background_padding_y;

                DrawStringBackgroundRectangle ( device, background_x1, background_y1, background_x2, background_y2,
                                                background_color );
            }
        }

        for ( const auto& captured : captured_strings )
        {
            const char* font_to_use = captured.font_name.empty () ? NULL : captured.font_name.c_str ();

            original_show_string_ex_function (
                captured.position_x, captured.position_y, captured.color, captured.text.c_str (), font_to_use,
                captured.font_size, captured.is_antialiased, captured.style, captured.second_color, captured.offset );
        }

        RestoreDirect3DRenderStates ( device, render_state_backup );

        captured_strings.clear ();
    }

    return original_end_scene_function ( device );
}

HRESULT WINAPI HookedReset ( LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* presentation_parameters )
{
    ImGui_ImplDX9_InvalidateDeviceObjects ();

    MH_DisableHook ( original_end_scene_address );
    is_imgui_initialized = false;

    HRESULT result = original_reset_function ( device, presentation_parameters );

    if ( SUCCEEDED ( result ) )
    {
        ImGui_ImplDX9_CreateDeviceObjects ();

        ImGui_ImplDX9_Shutdown ();
        ImGui_ImplWin32_Shutdown ();
        ImGui::DestroyContext ();

        MH_EnableHook ( original_end_scene_address );
    }

    return result;
}

static LRESULT CALLBACK HookedWindowProcedure ( HWND window_handle, UINT message, WPARAM w_param, LPARAM l_param )
{
    if ( is_imgui_initialized && game_window.is_gui_window_open )
    {
        ImGuiIO& io = ImGui::GetIO ();

        ImGui_ImplWin32_WndProcHandler ( window_handle, message, w_param, l_param );

        if ( io.WantCaptureMouse || io.WantCaptureKeyboard )
        {
            switch ( message )
            {
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

    return CallWindowProcA ( game_window.original_window_procedure, window_handle, message, w_param, l_param );
}

void InstallWindowProcedureHook ()
{
    while ( !game_window.game_window_handle )
    {
        game_window.game_window_handle = FindGameWindowHandle ();

        if ( game_window.game_window_handle && !game_window.original_window_procedure )
        {
            game_window.original_window_procedure = (WNDPROC)SetWindowLongA (
                game_window.game_window_handle, GWLP_WNDPROC, (LONG_PTR)HookedWindowProcedure );
        }
        Sleep ( 100 );
    }
}
