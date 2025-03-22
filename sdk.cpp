#include "sdk.h"

std::uintptr_t c_csgo::csgo_entitylist()
{
    if (!vars::cs2_entitylist || !vars::module_client)
    {
        this->csgo_client();
        return vars::cs2_entitylist = memory.Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwEntityList);
    }

    return vars::cs2_entitylist;
}

std::uintptr_t c_csgo::csgo_client()
{
    vars::module_client = memory.GetModuleAddress(L"client.dll");
    return vars::module_client;
}

std::uintptr_t c_csgo::csgo_engine2()
{
    vars::module_engine2 = memory.GetModuleAddress(L"engine2.dll");
    return vars::module_engine2;
}

int c_csgo::csgo_buildnumber()
{
    if (!vars::module_engine2) this->csgo_engine2();
    vars::cs2_buildnumber = memory.Read<int>(vars::module_engine2 + cs2_dumper::offsets::engine2_dll::dwBuildNumber);
    return vars::cs2_buildnumber;
}

cs_window c_csgo::csgo_window()
{
    if (!vars::module_engine2) this->csgo_engine2();
  
    vars::cs2_window.windowwidth = memory.Read<int>(vars::module_engine2 + cs2_dumper::offsets::engine2_dll::dwWindowWidth);
    vars::cs2_window.windowheight = memory.Read<int>(vars::module_engine2 + cs2_dumper::offsets::engine2_dll::dwWindowHeight);
     
    return vars::cs2_window;
}

void c_csgo::updateIGlobalVars()
{
    memory.ReadRaw(memory.Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwGlobalVars), &IGlobalVars, sizeof(C_IGlobalVars));
}

int c_csgo::csgo_highestentity()
{
    return memory.Read<int>(vars::cs2_entitylist + cs2_dumper::offsets::client_dll::dwGameEntitySystem_highestEntityIndex);
}

bool c_csgo::csgo_update()
{
    std::uintptr_t old_client = vars::module_client;
    std::uintptr_t old_engine2 = vars::module_engine2;
    int old_buildnumber = vars::cs2_buildnumber;
    int old_width = vars::cs2_window.windowwidth;
    int old_height = vars::cs2_window.windowheight;

    
    if (!vars::module_client)
    {
        vars::module_client = this->csgo_client();
    }

    if (!vars::cs2_entitylist)
    {
        vars::cs2_entitylist = this->csgo_entitylist();
    }

    if (!vars::module_engine2)
    {
        vars::module_engine2 = this->csgo_engine2();
    }

    this->csgo_buildnumber();
    this->csgo_window();
    this->csgo_highestentity();
    this->updateIGlobalVars();

    return true;
}

void c_csgo::csgo_force_cache()
{
    vars::module_client = this->csgo_client();
    vars::cs2_entitylist = this->csgo_entitylist();
    vars::module_engine2 = this->csgo_engine2();
    this->csgo_buildnumber();
    this->csgo_window();
}



