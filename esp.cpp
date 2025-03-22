#include "esp.h"


void cesp::renderBoxEsp(C_BaseEntity CBaseEntity)
{
    Vector_t basePosition = memory.Read<Vector_t>(CBaseEntity.Address + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
    Vector_t headPosition = { basePosition.x, basePosition.y, basePosition.z + 72.5f };

    Vector_t w2sHead = world_to_screen(headPosition);
    Vector_t w2sBase = world_to_screen(basePosition);

     if (w2sHead.IsZero() || w2sBase.IsZero()) return; 

    Vector2D_t base = w2sBase.ToVector2D();
    Vector2D_t top = w2sHead.ToVector2D();

    float height = base.y - top.y;
    float width = height * 0.55f;

    ImGui::GetBackgroundDrawList()->AddRect(
        { top.x - width / 2, top.y },
        { top.x + width / 2, base.y },
        IM_COL32(225, 255, 255, 155)
    );
}

void cesp::renderBoxEsp(C_BaseEntity CBaseEntity, BoxEspParams& boxParam)
{
    float height = boxParam.base.y - boxParam.top.y;
    float width = height * boxParam.width;

    ImGui::GetBackgroundDrawList()->AddRect(
        { boxParam.top.x - width / 2, boxParam.top.y },
        { boxParam.top.x + width / 2, boxParam.base.y },
        IM_COL32(225, 255, 255, 155)
    );
}

void cesp::think()
{
	vars::view_matrix = memory.Read<view_matrix_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewMatrix);

    for (const auto& CEntityInstance : entitysystem.CEntityInstances)
    {
        if (CEntityInstance.CEntityIdentity.m_designerName == "cs_player_controller") {
            uintptr_t m_hPawn = GetEntityAddress(memory.Read<uintptr_t>(CEntityInstance.Address + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn));
            if (!m_hPawn) 
                continue;
            
            C_BaseEntity BaseEntity{ m_hPawn };
            if (!BaseEntity.Update())
                continue;

            if (BaseEntity.IsDead())
                continue;
            
            if (BaseEntity.m_iTeamNum == entitysystem.CLocalPlayer.m_iTeamNum)
                continue;

            if (config.cs_player_controller_esp) {
                renderBoxEsp(BaseEntity);

            }
        }

        if (CEntityInstance.CEntityIdentity.m_designerName == "chicken") {
            uintptr_t m_pGameSceneNode = memory.Read<uintptr_t>(CEntityInstance.Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

            Vector_t basePosition = memory.Read<Vector_t>(m_pGameSceneNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
            Vector_t topPosition = { basePosition.x, basePosition.y, basePosition.z + 20.f };

            Vector_t w2sTop = world_to_screen(topPosition);
            Vector_t w2sBase = world_to_screen(basePosition);

            C_BaseEntity BaseEntity{ CEntityInstance.Address };
            if (!BaseEntity.Update())
                continue;

            cesp::BoxEspParams BoxParams;
            BoxParams.base = w2sBase.ToVector2D();
            BoxParams.top = w2sTop.ToVector2D();
            BoxParams.width = 1.2f;


            if (config.chicken_esp) {
                renderBoxEsp(BaseEntity, BoxParams);
            }

        }

    }
}