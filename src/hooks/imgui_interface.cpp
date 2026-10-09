#include "imgui_interface.h"
#include "directx_hooks.h"
#include "string_editor.h"
#include "always_jump.h"
#include "chams.h"
#include "imgui.h"

void RenderImGuiInterface ()
{
    if ( !game_window.is_gui_window_open )
    {
        return;
    }

    ImGui::SetNextWindowSize ( ImVec2 ( 350, 400 ), ImGuiCond_FirstUseEver );
    ImGui::SetNextWindowPos ( ImVec2 ( 50, 50 ), ImGuiCond_FirstUseEver );

    ImGui::Begin ( "ConquerDX9.Hook by Carniato", nullptr, ImGuiWindowFlags_None );

    UpdateRainbowColors ();

    ImGui::Text ( "Game Features" );
    ImGui::Separator ();
    ImGui::Checkbox ( "Always Jump", &is_always_jump_enabled );
    ImGui::Checkbox ( "Wireframe (Chams)", &is_wireframe_enabled );

    ImGui::Spacing ();

    ImGui::Text ( "String Modifications" );
    ImGui::Separator ();
    ImGui::Checkbox ( "Server Name Rainbow", &server_name_use_rainbow );

    ImGui::Spacing ();

    ImGui::Text ( "Item Strings" );
    ImGui::Separator ();

    ImGui::BeginChild ( "ItemStrings", ImVec2 ( 0, 200 ), true );

    for ( size_t i = 0; i < item_string_configurations.size (); i++ )
    {
        ImGui::PushID ( static_cast<int> ( i ) );
        StringConfiguration& config = item_string_configurations[i];

        ImGui::Checkbox ( "Rainbow", &config.use_rainbow_color );
        ImGui::SameLine ();
        ImGui::Checkbox ( "Background", &config.show_background );

        if ( !config.use_rainbow_color )
        {
            ImGui::Checkbox ( "Custom Color", &config.use_custom_text_color );
            if ( config.use_custom_text_color )
            {
                ImGui::ColorEdit4 ( "Color", &config.text_color_red );
            }
        }

        ImGui::Spacing ();
        ImGui::PopID ();
    }
    ImGui::EndChild ();

    ImGui::End ();
}
