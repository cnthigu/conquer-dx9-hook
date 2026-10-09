#pragma once

#include <windows.h>
#include <d3d9.h>
#include <cstdint>
#include <string>
#include <vector>

typedef HRESULT ( WINAPI* EndSceneFunc ) ( LPDIRECT3DDEVICE9 );
typedef HRESULT ( WINAPI* ResetFunc ) ( LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS* );
typedef BOOL ( WINAPI* GetKeyboardStateFunc ) ( PBYTE );
typedef HRESULT ( WINAPI* DrawIndexedPrimitiveFunc ) ( LPDIRECT3DDEVICE9, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT,
                                                       UINT );

struct Size2D
{
    int width;
    int height;
};

struct Position2D
{
    int x;
    int y;
};

enum TextRenderStyle
{
    TEXT_STYLE_DEFAULT = 0,
    TEXT_STYLE_BOLD = 1,
};

typedef Size2D ( __cdecl* ShowStringExFunc ) ( int, int, unsigned long, const char*, const char*, int, bool,
                                               TextRenderStyle, unsigned long, Position2D );

struct StringConfiguration
{
    std::string search_string;
    bool is_enabled = true;
    bool use_custom_text_color = false;
    bool use_rainbow_color = false;
    float text_color_red = 1.0f;
    float text_color_green = 1.0f;
    float text_color_blue = 1.0f;
    float text_color_alpha = 1.0f;
    bool show_background = true;

    StringConfiguration () = default;

    StringConfiguration ( const std::string& text ) : search_string ( text )
    {
    }
};

struct CapturedStringData
{
    std::string text;
    std::string font_name;
    std::string matched_search_string;
    int position_x = 0;
    int position_y = 0;
    DWORD color = 0;
    int font_size = 0;
    bool is_antialiased = false;
    TextRenderStyle style = TEXT_STYLE_DEFAULT;
    DWORD second_color = 0;
    Position2D offset = { 0, 0 };
};

struct GameWindowInfo
{
    HWND parent_window_handle = NULL;
    HWND game_window_handle = NULL;
    WNDPROC original_window_procedure = NULL;
    LPDIRECT3DDEVICE9 direct3d_device = nullptr;
    bool is_gui_window_open = true;
};

struct Direct3DRenderStateBackup
{
    DWORD flexible_vertex_format;
    DWORD lighting_state;
    DWORD z_buffer_enable;
    DWORD z_buffer_write_enable;
    DWORD alpha_blend_enable;
    DWORD source_blend_mode;
    DWORD destination_blend_mode;
    DWORD cull_mode;
};

constexpr float text_width_multiplier = 0.6f;
constexpr int background_padding_x = 4;
constexpr int background_padding_y = 2;
constexpr int background_alpha = 200;
constexpr size_t max_captured_strings = 1000;
