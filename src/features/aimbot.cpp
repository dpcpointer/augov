#include "aimbot.h"

Vector_t caimbot::GetViewAngels() {
    return m.read<Vector_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewAngles);
}

void caimbot::SetCameraPos(Vector_t Position) {
    INPUT input = {};
    switch (config.Mode) {
    case AimbotMode::Mouse:
        input.type = INPUT_MOUSE;
        input.mi.dx = static_cast<LONG>(Position.x * config.aimAmt); 
        input.mi.dy = static_cast<LONG>(Position.y * config.aimAmt);
        input.mi.mouseData = 0;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.time = 0;
        input.mi.dwExtraInfo = 0;
        NtUserSendInput(1, &input, sizeof(INPUT));
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        break;

    case AimbotMode::None:
        break;

    default: break;
    }
}


void caimbot::think() {
    if (!NtUserSendInput) {
        NtUserSendInput = (NtUserSendInput_t)GetProcAddress(GetModuleHandleA("win32u"), "NtUserSendInput");
    }
    bool keyPressed = GetAsyncKeyState((int)config.activationKey) & 0x8000;
    Vector_t eyePos, angles;
    Vector_t punchAngle, punchAngleVel;
    int shotsFired;
    int punchTickBase;
    float punchTickFraction;
    uintptr_t punchCacheAddr;
    int punchCacheCount;

    uintptr_t localPawn = entitysystem.CLocalPlayer.Address;

    if (config.recoilControlSystem) {
        shotsFired = m.read<int>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);
        punchAngle = m.read<Vector_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);
        punchAngleVel = m.read<Vector_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngleVel);
        punchTickBase = m.read<int>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchTickBase);
        punchTickFraction = m.read<float>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchTickFraction);
        punchCacheAddr = m.read<uintptr_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache);
        punchCacheCount = m.read<int>(punchCacheAddr + 0x8); 
    }


    Vector_t bestTargetPos;
    bool found = false;

    float screenCenterX = static_cast<float>(overlay::G_Width) / 2;
    float screenCenterY = static_cast<float>(overlay::G_Height) / 2;
    float fovRadius = config.fieldOfView;
    float closestDistanceToCenter = FLT_MAX;

    for (auto& entity : entitysystem.CBasePlayerEntities) {
        if (!entity.Address || entity.IsDead() || entity.m_iTeamNum == entitysystem.CLocalPlayer.m_iTeamNum)
            continue;

        uintptr_t boneArray = m.read<uintptr_t>(entity.CGameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
        Vector_t headPos = m.read<Vector_t>(boneArray + 6 * 32);

        Vector2D_t screenPos = world_to_screen(headPos).ToVector2D();
        if (screenPos.IsZero())
            continue;

        float dx = screenPos.x - screenCenterX;
        float dy = screenPos.y - screenCenterY;
        float distanceToCenter = sqrtf(dx * dx + dy * dy);

        if (distanceToCenter <= fovRadius && distanceToCenter < closestDistanceToCenter) {
            bestTargetPos = headPos;
            closestDistanceToCenter = distanceToCenter;
            found = true;
        }
    }

    if (found && keyPressed) {
         if (config.Mode == AimbotMode::Mouse) {
            Vector2D_t screenHead = world_to_screen(bestTargetPos).ToVector2D();
            if (!screenHead.IsZero()) {
                float deltaX = (screenHead.x - screenCenterX);
                float deltaY = (screenHead.y - screenCenterY);
                if (config.recoilControlSystem && shotsFired > 1) {
                    float rcsX = punchAngle.y * config.recoilControlSystemX;
                    float rcsY = punchAngle.x * config.recoilControlSystemY;

                    rcsX += punchAngleVel.y * punchTickFraction * 0.1f;
                    rcsY += punchAngleVel.x * punchTickFraction * 0.1f;

                    if (punchCacheCount > 0 && punchCacheCount <= 0xFFFF) {
                        Vector_t lastPunch = m.read<Vector_t>(punchCacheAddr + 0x10 + (punchCacheCount - 1) * sizeof(Vector_t));
                        rcsX = (rcsX + lastPunch.y * config.recoilControlSystemX) * 0.5f;
                        rcsY = (rcsY + lastPunch.x * config.recoilControlSystemY) * 0.5f;
                    }

                    deltaX -= rcsX;
                    deltaY -= rcsY;
                }
                SetCameraPos({ deltaX, deltaY, 0 });
            }
        }
    }
}


void ctriggerbot::think()
{
    if (!config.TriggerBot) return; 

    int m_iIDEntIndex = m.read<int>(entitysystem.CLocalPlayer.Address +
        cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_iIDEntIndex);

    if (m_iIDEntIndex <= 0) return; 

    C_BaseEntity BaseEntity;
    BaseEntity.Address = GetEntity(m_iIDEntIndex).Address;
    if (!BaseEntity.Update()) return;
    if (BaseEntity.IsDead()) return;
    if (BaseEntity.m_iTeamNum == entitysystem.CLocalPlayer.m_iTeamNum) return;

    if (GetAsyncKeyState(config.triggeractivationKey) & 0x8000)
    {
        INPUT input = { 0 };
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        aimbot.NtUserSendInput(1, &input, sizeof(INPUT));
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        aimbot.NtUserSendInput(1, &input, sizeof(INPUT));
    }
}