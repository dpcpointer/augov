#ifndef SDK_HPP
#define SDK_HPP
#include "include.hpp"

inline class C_IGlobalVars
{
public:
	float m_flRealTime; //0x0000
	int32_t m_nFrameCount; //0x0004
	float m_flFrameTime; //0x0008
	float m_flFrameTime2; //0x000C
	int32_t m_nMaxClients; //0x0010
private:
	char pad0x1c[0x1C];
public:
	float m_flIntervalPerSubTick; //0x0030
	float m_flCurrentTime; //0x0034
	float m_flCurrentTime2; //0x0038
private:
	char pad0x14[0x14];
public:
	int32_t m_nTickCount; //0x0048
}IGlobalVars;

inline class c_csgo
{
public:
    std::uintptr_t csgo_client();
    std::uintptr_t csgo_engine2();
    std::uintptr_t csgo_entitylist();
    int csgo_buildnumber();
    cs_window csgo_window();
    int csgo_highestentity();
	void updateIGlobalVars();
    bool csgo_update();
    void csgo_force_cache();
}csgo;

constexpr std::uint32_t MAX_ENTITIES_IN_LIST = 512;
constexpr std::uint32_t MAX_ENTITY_LISTS = 64;
constexpr std::uint32_t MAX_TOTAL_ENTITIES = MAX_ENTITIES_IN_LIST * MAX_ENTITY_LISTS;

class C_CEntityIdentity {
public:
    uintptr_t Address = 0;
    std::string m_designerName;

    bool get_m_designerName() {
        if (!Address)
            return false;

        uintptr_t DesignerNameAddress = memory->Read<uintptr_t>(Address + cs2_dumper::schemas::client_dll::CEntityIdentity::m_designerName);
        if (!DesignerNameAddress)
            return false;

        char NameBuffer[MAX_PATH] = {};
        if (!memory->ReadRaw(DesignerNameAddress, NameBuffer, sizeof(NameBuffer)))
            return false;

        m_designerName = std::string(NameBuffer);
        return !m_designerName.empty();
    }

    bool Update() {
        if (!Address)
            return false;
        return get_m_designerName();
    }
};

class C_CEntityInstance {
public:
    uintptr_t Address = 0;
    C_CEntityIdentity CEntityIdentity;

    bool Update() {
        if (!Address)
            return false;

        CEntityIdentity.Address = memory->Read<uintptr_t>(Address + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity);
        if (!CEntityIdentity.Address)
            return false;

        return CEntityIdentity.Update();
    }
};

static C_CEntityInstance GetEntity(int nIdx) noexcept {
    C_CEntityInstance entity;
    uintptr_t uListEntry = memory->Read<uintptr_t>(vars::cs2_entitylist + 8LL * ((nIdx & 0x7FFF) >> 9) + 16);
    if (!uListEntry)
        return entity;

    entity.Address = memory->Read<uintptr_t>(uListEntry + 120LL * (nIdx & 0x1FF));
    return entity;
}

static uintptr_t GetEntityAddress(uintptr_t address) noexcept {
    uintptr_t uListEntry = memory->Read<uintptr_t>(vars::cs2_entitylist + 0x8 * ((address & 0x7FFF) >> 9) + 16);
    if (!uListEntry)
        return 0;

    return memory->Read<uintptr_t>(uListEntry + 120LL * (address & 0x1FF));
}

class C_BaseEntity : public C_CEntityInstance
{
public:
    uintptr_t Address;
    uintptr_t CGameSceneNode;

    int m_iHealth;
    int m_lifeState;
    bool m_bTakesDamage;
    float m_flSpeed;
    int m_iTeamNum;
    int m_spawnflags;
    int m_fFlags;
    Vector_t m_vecAbsVelocity;

    bool Update()
    {
        if (!Address)
            return false;

         this->CGameSceneNode = memory->Read<uintptr_t>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
         if (!this->CGameSceneNode)
             return false;

         this->m_iHealth = memory->Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
         this->m_lifeState = memory->Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_lifeState);
         this->m_bTakesDamage = memory->Read<bool>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_bTakesDamage);
         this->m_flSpeed = memory->Read<float>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_flSpeed);
         this->m_iTeamNum = memory->Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
         this->m_spawnflags = memory->Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_spawnflags);
         this->m_vecAbsVelocity = memory->Read<Vector_t>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_vecAbsVelocity);
        return true;
    }
};

inline class c_entitysystem
{
public:
    std::vector<C_CEntityInstance> CEntityInstances;
    C_CEntityInstance CLocalPlayer;

    std::chrono::steady_clock::time_point LastLoopTime = std::chrono::steady_clock::now();
    int CEntityInstancesCount = 0;

    void think() {
        auto CurrentTime = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(CurrentTime - LastLoopTime).count() >= 4) {
            LastLoopTime = CurrentTime;
            CEntityInstancesCount = 0;

            csgo.csgo_update(); // update vars

            CLocalPlayer.Address = memory->Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
            if (!CLocalPlayer.Update())
            {
                CLocalPlayer.Address = 0;
            }

            CEntityInstances.resize(MAX_TOTAL_ENTITIES + 1);

            for (int idx = 0; idx < MAX_TOTAL_ENTITIES; idx++) {
                C_CEntityInstance temp = GetEntity(idx);
                if (temp.Address == 0) {
                    CEntityInstances[idx].Address = 0;
                }
                else {
                    CEntityInstances[idx].Address = temp.Address;
                    if (!CEntityInstances[idx].Update()) {
                        CEntityInstances[idx].Address = 0;
                    }
                    else {
                        CEntityInstancesCount++;
                    }
                }
            }
        }
    }

}entitysystem;


#endif



