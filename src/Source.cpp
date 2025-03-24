#pragma once
#include "include.hpp"
#include "features/esp.h"
#include "features/aimbot.h"
#include "game/sdk.h"
#include "features/config.h"
#include "overlay/font.hpp"

int main() {
	if (!memory.Attach(L"cs2.exe")) {
		return -1;
	}


	if (!overlay::SetupOverlay())
	{
		memory.Detach();
		return -1;
	}

	ImGuiIO& io = ImGui::GetIO();

	static ImWchar Ranges[] = {
		0x0020, 0xFFFF, // Unicode U+0020 to U+FFFF
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x017F, // Latin Extended-A
		0x0180, 0x024F, // Latin Extended-B
		0x0250, 0x02AF, // IPA Extensions
		0x0370, 0x03FF, // Greek and Coptic
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0xA640, 0xA69F, // Cyrillic Extended-B
		0x0590, 0x05FF, // Hebrew
		0x0600, 0x06FF, // Arabic
		0x0900, 0x097F, // Devanagari
		0x2000, 0x206F, // General Punctuation
		0x20A0, 0x20CF, // Currency Symbols
		0x2100, 0x214F, // Letterlike Symbols
		0x2200, 0x22FF, // Mathematical Operators
		0x2300, 0x23FF, // Miscellaneous Technical
		0x2500, 0x257F, // Box Drawing
		0x25A0, 0x25FF, // Geometric Shapes
		0x2600, 0x26FF, // Miscellaneous Symbols
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0x4E00, 0x9FAF, // CJK Ideograms
		0xFF00, 0xFFEF, // Half-width characters
		0x1F300, 0x1F5FF, // Miscellaneous Symbols and Pictographs
		0x1F600, 0x1F64F, // Emoticons
		0x1F680, 0x1F6FF, // Transport and Map Symbols
		0x1F700, 0x1F77F, // Alchemical Symbols
		0x1F900, 0x1F9FF, // Supplemental Symbols and Pictographs
		0,
	};

	ImFontConfig FontConfig;
	FontConfig.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags::ImGuiFreeTypeBuilderFlags_MonoHinting;
	FontConfig.OversampleH = 2;
	FontConfig.OversampleV = 2;

	io.Fonts->Clear();

	const float baseSize = 10.0f;
	const float sizeIncrement = 2.0f;
	constexpr int fontCount = 6;

	for (int i = 0; i < fontCount; i++)
	{
		float fontSize = baseSize + (i * sizeIncrement); // 10, 12, 14, 16, 18, 20

		esp.FontScallingArray[i] = io.Fonts->AddFontFromMemoryTTF(
			CASCADIAMONO,
			sizeof(CASCADIAMONO),
			fontSize,
			nullptr,
			io.Fonts->GetGlyphRangesDefault()
		);

		if (!esp.FontScallingArray[i])
		{
			return -1;
		}
	}

	io.FontDefault = io.Fonts->AddFontFromMemoryTTF(CASCADIAMONO, sizeof(CASCADIAMONO), 18.5f);
	io.Fonts->Build();

	csgo.csgo_update();



	printf("[augov] module [client.dll] - %lld\n", vars::module_client);
	printf("[augov] module [engine2.dll] - %lld\n", vars::module_engine2);
	printf("[augov] dwEntityList - %lld\n", vars::cs2_entitylist);
	printf("[augov] game version - %ld\n", vars::cs2_buildnumber);
	printf("[augov] game pid - %lld\n", memory.processId);
	printf("[augov] game handle - %p\n", memory.processHandle);
	printf("[augov] attached \n");

	while (!overlay::ShouldQuit) {

		overlay::Render();
		entitysystem.think();
		esp.think();
		aimbot.think();

		ImGui::PushFont(ImGui::GetDefaultFont());

		ImGui::Begin("menu");

		ImGui::Checkbox("player esp", &config.cs_player_controller_esp);
		ImGui::Checkbox("health bar", &config.cs_player_controller_health);
		ImGui::Checkbox("health text", &config.cs_player_controller_healthtext);
		ImGui::Checkbox("skeleton", &config.cs_player_controller_skeleton);
		ImGui::Checkbox("chicken esp", &config.chicken_esp);

		const char* modes[] = { "Silent", "Memory", "Mouse", "None" };
		static int currentMode = static_cast<int>(config.Mode);
		if (ImGui::Combo("Aimbot Mode", &currentMode, modes, IM_ARRAYSIZE(modes))) {
			config.Mode = static_cast<AimbotMode>(currentMode);
		}



		ImGui::End();

		ImGui::PopFont();






		overlay::EndRender();
	}

	memory.Detach();
	overlay::CloseOverlay();
	return 0;
}