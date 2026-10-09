#include "string_editor.h"
#include <cmath>
#include "MinHook.h"

ShowStringExFunc original_show_string_ex_function = nullptr;
std::vector<StringConfiguration> item_string_configurations;
std::vector<CapturedStringData> captured_strings;
bool server_name_use_rainbow = false;

static std::string server_name = "WarLord";
static bool server_name_enabled = true;
static bool server_name_use_custom_color = false;
static float server_name_color_red = 1.0f;
static float server_name_color_green = 1.0f;
static float server_name_color_blue = 1.0f;
static float server_name_color_alpha = 1.0f;

static float rainbow_speed = 1.0f;
static float rainbow_red = 0.0f;
static float rainbow_green = 0.0f;
static float rainbow_blue = 0.0f;
static float rainbow_time = 0.0f;
static DWORD rainbow_last_tick = 0;

static LPVOID original_show_string_ex_address = nullptr;
static bool is_show_string_hook_installed = false;

void UpdateRainbowColors ()
{
    DWORD current_tick = GetTickCount ();

    if ( rainbow_last_tick == 0 )
    {
        rainbow_last_tick = current_tick;
        return;
    }

    float delta_time = ( current_tick - rainbow_last_tick ) / 1000.0f;
    rainbow_last_tick = current_tick;

    rainbow_time += delta_time * rainbow_speed;

    rainbow_red = ( sin ( rainbow_time + 0.0f ) + 1.0f ) / 2.0f;
    rainbow_green = ( sin ( rainbow_time + 2.0f ) + 1.0f ) / 2.0f;
    rainbow_blue = ( sin ( rainbow_time + 4.0f ) + 1.0f ) / 2.0f;
}

static DWORD GetRainbowColor ()
{
    return D3DCOLOR_RGBA ( static_cast<int> ( rainbow_red * 255.0f + 0.5f ),
                           static_cast<int> ( rainbow_green * 255.0f + 0.5f ),
                           static_cast<int> ( rainbow_blue * 255.0f + 0.5f ), 255 );
}

const StringConfiguration* FindItemStringConfiguration ( const std::string& matched_string )
{
    for ( const auto& config : item_string_configurations )
    {
        if ( config.search_string == matched_string )
        {
            return &config;
        }
    }
    return nullptr;
}

static bool IsServerNameString ( const char* string_text )
{
    if ( !string_text || server_name.empty () )
    {
        return false;
    }
    return strstr ( string_text, server_name.c_str () ) != nullptr;
}

static Size2D __cdecl HookedShowStringEx ( int position_x, int position_y, unsigned long color, const char* string_text,
                                           const char* font_name, int font_size, bool is_antialiased,
                                           TextRenderStyle style, unsigned long second_color, Position2D offset )
{
    if ( !original_show_string_ex_function )
    {
        return Size2D{ 0, 0 };
    }

    if ( !string_text )
    {
        return original_show_string_ex_function ( position_x, position_y, color, string_text, font_name, font_size,
                                                  is_antialiased, style, second_color, offset );
    }

    bool is_server_name = IsServerNameString ( string_text );

    StringConfiguration* matching_item_config = nullptr;
    for ( auto& config : item_string_configurations )
    {
        if ( config.is_enabled && strstr ( string_text, config.search_string.c_str () ) != nullptr )
        {
            matching_item_config = &config;
            break;
        }
    }

    bool should_capture = false;
    bool should_use_rainbow = false;
    bool should_use_custom_color = false;
    DWORD custom_color = color;
    std::string matched_string = "";

    if ( is_server_name && server_name_enabled )
    {
        should_capture = true;
        matched_string = "SERVER_NAME";

        if ( server_name_use_rainbow )
        {
            should_use_rainbow = true;
        }
        else if ( server_name_use_custom_color )
        {
            should_use_custom_color = true;
            custom_color = D3DCOLOR_RGBA ( static_cast<int> ( server_name_color_red * 255.0f + 0.5f ),
                                           static_cast<int> ( server_name_color_green * 255.0f + 0.5f ),
                                           static_cast<int> ( server_name_color_blue * 255.0f + 0.5f ),
                                           static_cast<int> ( server_name_color_alpha * 255.0f + 0.5f ) );
        }
    }
    else if ( matching_item_config )
    {
        should_capture = true;
        matched_string = matching_item_config->search_string;

        if ( matching_item_config->use_rainbow_color )
        {
            should_use_rainbow = true;
        }
        else if ( matching_item_config->use_custom_text_color )
        {
            should_use_custom_color = true;
            custom_color =
                D3DCOLOR_RGBA ( static_cast<int> ( matching_item_config->text_color_red * 255.0f + 0.5f ),
                                static_cast<int> ( matching_item_config->text_color_green * 255.0f + 0.5f ),
                                static_cast<int> ( matching_item_config->text_color_blue * 255.0f + 0.5f ),
                                static_cast<int> ( matching_item_config->text_color_alpha * 255.0f + 0.5f ) );
        }
    }

    if ( should_capture )
    {
        CapturedStringData captured;
        captured.text = string_text;
        captured.font_name = font_name ? font_name : "";
        captured.position_x = position_x;
        captured.position_y = position_y;

        if ( should_use_rainbow )
        {
            captured.color = GetRainbowColor ();
        }
        else if ( should_use_custom_color )
        {
            captured.color = custom_color;
        }
        else
        {
            captured.color = color;
        }

        captured.font_size = font_size;
        captured.is_antialiased = is_antialiased;
        captured.style = style;
        captured.second_color = second_color;
        captured.offset = offset;
        captured.matched_search_string = matched_string;

        if ( captured_strings.size () < max_captured_strings )
        {
            captured_strings.push_back ( captured );
        }

        return Size2D{ 0, 0 };
    }

    return original_show_string_ex_function ( position_x, position_y, color, string_text, font_name, font_size,
                                              is_antialiased, style, second_color, offset );
}

void InstallShowStringExHook ()
{
    if ( is_show_string_hook_installed )
    {
        return;
    }

    HMODULE graphic_module_handle = GetModuleHandleA ( "graphic.dll" );

    if ( !graphic_module_handle )
    {
        return;
    }

    const char* show_string_ex_mangled_name =
        "?ShowStringEx@CMyBitmap@@SA?AUC3_SIZE@@HHKPBD0H_NW4RENDER_TEXT_STYLE@@KUC3_POS@@@Z";

    FARPROC show_string_ex_address = GetProcAddress ( graphic_module_handle, show_string_ex_mangled_name );

    if ( show_string_ex_address )
    {
        original_show_string_ex_address = (LPVOID)show_string_ex_address;
        original_show_string_ex_function = (ShowStringExFunc)show_string_ex_address;

        if ( MH_CreateHook ( original_show_string_ex_address, (LPVOID)HookedShowStringEx,
                             (LPVOID*)&original_show_string_ex_function ) == MH_OK )
        {
            MH_EnableHook ( original_show_string_ex_address );
            is_show_string_hook_installed = true;
        }
    }
}
