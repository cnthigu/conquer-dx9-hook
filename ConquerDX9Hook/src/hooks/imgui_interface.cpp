#include "imgui.h"
#include "common.h"

extern GameWindowInfo g_gameWindow;
extern bool g_isAlwaysJumpEnabled;
extern bool g_isWireframeEnabled;
extern std::vector<StringConfiguration> g_itemStringConfigurations;
extern bool g_serverNameEnabled;
extern bool g_serverNameUseRainbow;
extern void UpdateRainbowColors();

void RenderImGuiInterface() 
{
	if (!g_gameWindow.isGuiWindowOpen) 
		return;

	ImGui::SetNextWindowSize(ImVec2(350, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_FirstUseEver);
	
	ImGui::Begin("ConquerDX9.Hook by Carniato", nullptr, ImGuiWindowFlags_None);

	UpdateRainbowColors();
	
	ImGui::Text("Game Features");
	ImGui::Separator();
	ImGui::Checkbox("Always Jump", &g_isAlwaysJumpEnabled);        
	ImGui::Checkbox("Wireframe (Chams)", &g_isWireframeEnabled);   
	
	ImGui::Spacing();
	
	ImGui::Text("String Modifications");
	ImGui::Separator();
	ImGui::Checkbox("Server Name Rainbow", &g_serverNameUseRainbow); 
	
	ImGui::Spacing();
	
	ImGui::Text("Item Strings");
	ImGui::Separator();

	ImGui::BeginChild("ItemStrings", ImVec2(0, 200), true);
	
	for (size_t i = 0; i < g_itemStringConfigurations.size(); i++) 
	{
		ImGui::PushID(static_cast<int>(i));
		StringConfiguration& config = g_itemStringConfigurations[i];
		
		ImGui::Checkbox("Rainbow", &config.useRainbowColor);
		ImGui::SameLine(); 
		ImGui::Checkbox("Background", &config.showBackground);
		
		if (!config.useRainbowColor) 
		{
			ImGui::Checkbox("Custom Color", &config.useCustomTextColor);
			if (config.useCustomTextColor) 
			{
				ImGui::ColorEdit4("Color", &config.textColorRed);
			}
		}
		
		ImGui::Spacing();  
		ImGui::PopID();
	}
	ImGui::EndChild();
	
	ImGui::End();
}
