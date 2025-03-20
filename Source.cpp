#include "sdk.hpp"
#include "SimpleOV.hpp"
#include <thread>
#include <chrono>

struct view_matrix_t {
    float* operator[](int index) { return matrix[index]; }
    float matrix[4][4];
} view_matrix;

Vector_t world_to_screen(const Vector_t& pos) {
    if (!view_matrix[0] || !view_matrix[1] || !view_matrix[3]) {
        return { 0, 0, 0 };
    }

    float _x = view_matrix[0][0] * pos.x + view_matrix[0][1] * pos.y + view_matrix[0][2] * pos.z + view_matrix[0][3];
    float _y = view_matrix[1][0] * pos.x + view_matrix[1][1] * pos.y + view_matrix[1][2] * pos.z + view_matrix[1][3];
    float w = view_matrix[3][0] * pos.x + view_matrix[3][1] * pos.y + view_matrix[3][2] * pos.z + view_matrix[3][3];

    if (w < 0.01f) {
        return { 0, 0, 0 };
    }

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
    if (!memory || !memory->Attach(L"cs2.exe")) {
        return -1;
    }

    overlay::SetupWindow();
    if (!overlay::Window || !overlay::CreateDeviceD3D(overlay::Window)) {
        memory->Detach();
        return -1;
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.Fonts->AddFontFromMemoryTTF(GenerisSimpleW04Heavyttf, sizeof(GenerisSimpleW04Heavyttf), 19.5f)) {
//io.Fonts->AddFontDefault();
        return -1;
    }

    csgo.csgo_update();

    while (!overlay::ShouldQuit) {
        
        overlay::Render();
        view_matrix = memory->Read<view_matrix_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewMatrix);
        entitysystem.think();

        C_BaseEntity LocalBaseEntity;
        LocalBaseEntity.Address = memory->Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
        if (!LocalBaseEntity.Update()) {
            continue;
        }

        ImGui::Begin("Debug##Default");
        ImGui::Text("Overlay Running");
        ImGui::End();

        for (const auto& CEntityInstance : entitysystem.CEntityInstances) {
            if (CEntityInstance.CEntityIdentity.m_designerName == "cs_player_controller") {
                uintptr_t m_hPawn = GetEntityAddress(memory->Read<uintptr_t>(
                    CEntityInstance.Address + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn));

                if (!m_hPawn) {
                    continue;
                }

                C_BaseEntity BaseEntity;
                BaseEntity.Address = m_hPawn;
                if (!BaseEntity.Update()) {
                    continue;
                }

                if (BaseEntity.m_iTeamNum == LocalBaseEntity.m_iTeamNum) {
                    continue;
                }

                uintptr_t BoneArrayAddress = memory->Read<uintptr_t>(
                    BaseEntity.CGameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);

                if (!BoneArrayAddress) {
                    continue;
                }

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

                bool bonesValid = true;
                for (int i = 0; i < 13; i++) {
                    if (bones[i].IsZero()) {
                        bonesValid = false;
                        break;
                    }
                }

                if (!bonesValid) {
                    continue;
                }

                struct BoneConnection {
                    int start;
                    int end;
                };

                constexpr int NUM_BONES = 13;
                static const BoneConnection connections[] = {
                    {0, 1}, {1, 2}, {1, 3}, {2, 4}, {3, 5},
                    {4, 6}, {5, 7}, {1, 8}, {8, 9}, {8, 10},
                    {9, 11}, {10, 12}
                };
                static const int NUM_CONNECTIONS = sizeof(connections) / sizeof(connections[0]);

                Vector_t screenBones[NUM_BONES];
                ImVec2 boneScreen[NUM_BONES];
                bool allVisible = true;

                for (int i = 0; i < NUM_BONES; ++i) {
                    screenBones[i] = world_to_screen(bones[i]);
                    if (screenBones[i].IsZero()) {
                        allVisible = false;
                        break;
                    }
                    boneScreen[i] = { screenBones[i].x, screenBones[i].y };
                }

                if (!allVisible) {
                    continue;
                }

                constexpr ImU32 COLOR_WHITE = IM_COL32(225, 255, 255, 255);
                constexpr float THICKNESS = 1.5f;

                for (int i = 0; i < NUM_CONNECTIONS; ++i) {
                    const auto& conn = connections[i];
                    ImGui::GetBackgroundDrawList()->AddLine(
                        boneScreen[conn.start],
                        boneScreen[conn.end],
                        COLOR_WHITE,
                        THICKNESS
                    );
                }
            }

            if (CEntityInstance.CEntityIdentity.m_designerName == "chicken")
            {
                uintptr_t gamescenenode = memory->Read<uintptr_t>(CEntityInstance.Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
                Vector_t ItemPos = memory->Read<Vector_t>(gamescenenode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
                Vector_t ItemPosW = world_to_screen(ItemPos);
                ImGui::GetBackgroundDrawList()->AddText({ ItemPosW.x, ItemPosW.y }, ImColor(255, 255, 255, 155), CEntityInstance.CEntityIdentity.m_designerName.c_str());
            }
          
        }

        overlay::EndRender();
    }

    memory->Detach();
    overlay::CloseOverlay();
    return 0;
}