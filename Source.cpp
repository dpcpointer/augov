#include "sdk.hpp"
#include "SimpleOV.hpp"

struct view_matrix_t {
    float* operator[](int index) { return matrix[index]; }
    float matrix[4][4];
} view_matrix;

Vector_t world_to_screen(const Vector_t& pos) {
    float _x = view_matrix[0][0] * pos.x + view_matrix[0][1] * pos.y + view_matrix[0][2] * pos.z + view_matrix[0][3];
    float _y = view_matrix[1][0] * pos.x + view_matrix[1][1] * pos.y + view_matrix[1][2] * pos.z + view_matrix[1][3];
    float w = view_matrix[3][0] * pos.x + view_matrix[3][1] * pos.y + view_matrix[3][2] * pos.z + view_matrix[3][3];

    float inv_w = w > 0.001f ? 1.f / w : 1.f;
    _x *= inv_w;
    _y *= inv_w;

    float screen_x = overlay::G_Width * 0.5f;
    float screen_y = overlay::G_Height * 0.5f;

    screen_x += 0.5f * _x * overlay::G_Width + 0.5f;
    screen_y -= 0.5f * _y * overlay::G_Height + 0.5f;

    return { screen_x, screen_y, w };
}

int main() {
    if (!memory->Attach(L"cs2.exe")) {
        return -1;
    }

    overlay::SetupWindow();
    if (!overlay::CreateDeviceD3D(overlay::Window)) {
        memory->Detach();
        return -1;
    }

    ImGui::GetIO().Fonts->AddFontFromMemoryTTF(GenerisSimpleW04Heavyttf, sizeof(GenerisSimpleW04Heavyttf), 15.5f);
    csgo.csgo_update();

    while (!overlay::ShouldQuit) {
        entitysystem.think();
        overlay::Render();
        view_matrix = memory->Read<view_matrix_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewMatrix);
        uintptr_t localplayerpawn = memory->Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
        int m_hPawn_local_teamid = memory->Read<int>(localplayerpawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

        for (const auto& CEntityInstance : entitysystem.CEntityInstances) {
            if (CEntityInstance.CEntityIdentity.m_designerName == "cs_player_controller") {
                uintptr_t m_hPawn = GetEntityAddress(memory->Read<uintptr_t>(
                    CEntityInstance.Address + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn));

                if (!m_hPawn)
                    continue;

                int m_hPawn_health = memory->Read<int>(m_hPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
                if (m_hPawn_health == 0)
                    continue;
                //
                int m_hPawn_teamid = memory->Read<int>(m_hPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);

                if (m_hPawn_local_teamid == m_hPawn_teamid)
                    continue;


                

                static ImColor BoneColor(255, 255, 255, 255);

                uintptr_t GameSceneNode = memory->Read<uintptr_t>(
                    m_hPawn + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);

                uintptr_t BoneArrayAddress = memory->Read<uintptr_t>(
                    GameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);

                enum BoneIndex {
                    HEAD = 6,
                    NECK = 5,
                    SPINE = 8,
                    LEFT_SHOULDER = 13,
                    LEFT_ELBOW = 9,
                    LEFT_HAND = 14,
                    RIGHT_SHOULDER = 11,
                    RIGHT_ELBOW = 16,
                    PELVIS = 0,
                    LEFT_HIP = 23,
                    LEFT_KNEE = 26,
                    RIGHT_HIP = 24,
                    RIGHT_KNEE = 27
                };

                Vector_t bones[13] = {
                    memory->Read<Vector_t>(BoneArrayAddress + HEAD * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + NECK * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + SPINE * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + LEFT_SHOULDER * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + LEFT_ELBOW * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + LEFT_HAND * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + RIGHT_SHOULDER * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + RIGHT_ELBOW * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + PELVIS * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + LEFT_HIP * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + LEFT_KNEE * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + RIGHT_HIP * 32),
                    memory->Read<Vector_t>(BoneArrayAddress + RIGHT_KNEE * 32)
                };

                Vector_t screenBones[13];
                bool allVisible = true;

                for (int i = 0; i < 13; ++i) {
                    screenBones[i] = world_to_screen(bones[i]);
                    if (screenBones[i].IsZero())
                        allVisible = false;
                }

                if (!allVisible)
                    continue;

                ImVec2 boneScreen[13];
                for (int i = 0; i < 13; ++i) {
                    boneScreen[i] = ImVec2(screenBones[i].x, screenBones[i].y);
                }

                BoneColor.Value.z = 255;
                auto drawBoneLine = [&](int a, int b) {
                    if (a < 13 && b < 13) {
                        ImGui::GetBackgroundDrawList()->AddLine(boneScreen[a], boneScreen[b], BoneColor, 2.f);
                    }
                };

                drawBoneLine(0, 1);
                drawBoneLine(1, 2);
                drawBoneLine(1, 3);
                drawBoneLine(2, 4);
                drawBoneLine(3, 5);
                drawBoneLine(4, 6);
                drawBoneLine(5, 7);
                drawBoneLine(1, 8);
                drawBoneLine(8, 9);
                drawBoneLine(8, 10);
                drawBoneLine(9, 11);
                drawBoneLine(10, 12);
            }
        }

        overlay::EndRender();
    }

    memory->Detach();
    overlay::CloseOverlay();
    return 0;
}