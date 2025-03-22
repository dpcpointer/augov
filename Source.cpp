#pragma once
#include "include.hpp"
#include "esp.h"
#include "sdk.h"
#include "config.h"

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
    if (!io.Fonts->AddFontFromMemoryTTF(GenerisSimpleW04Heavyttf, sizeof(GenerisSimpleW04Heavyttf), 19.5f)) {
        return -1;
    }

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


        ImGui::Begin("menu");

        ImGui::Checkbox("player esp", &config.cs_player_controller_esp);
        ImGui::Checkbox("chicken esp", &config.chicken_esp);


        ImGui::End();

       
       
      
        
       
        overlay::EndRender();
    }

    memory.Detach();
    overlay::CloseOverlay();
    return 0;
}