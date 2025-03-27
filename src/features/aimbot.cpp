#include "aimbot.h"

Vector_t caimbot::GetViewAngels() {
    return memory.Read<Vector_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewAngles);
}

void caimbot::SetCameraPos(Vector_t Position) {
    INPUT input = {};
    switch (config.Mode) {
    case AimbotMode::Silent:
        if (!pSilentHookInitialized) return;
        SilentAimValues[0] = Position.x;
        SilentAimValues[1] = Position.y;
        memory.WriteRaw((uintptr_t)pSilentAimValues, SilentAimValues, sizeof(SilentAimValues));
        break;

    case AimbotMode::Memory:
        memory.Write<Vector2D_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewAngles, Position.ToVector2D());
        break;

    case AimbotMode::Mouse:
        input.type = INPUT_MOUSE;
        input.mi.dx = static_cast<LONG>(Position.x * 2.5f); 
        input.mi.dy = static_cast<LONG>(Position.y * 2.5f); 
        input.mi.mouseData = 0;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        input.mi.time = 0;
        input.mi.dwExtraInfo = 0;
        NtUserSendInput(1, &input, sizeof(INPUT));
        break;

    case AimbotMode::None:
        break;
    }
}

void caimbot::CleanupSilentHook() {
    if (pSilentHookInitialized) {
        memory.WriteRaw(vars::module_client + SILENT_OG, hk_orig, sizeof(hk_orig));
        if (pSilentAimHook) VirtualFreeEx(memory.processHandle, pSilentAimHook, 0, MEM_RELEASE);
        if (pSilentAimValues) VirtualFreeEx(memory.processHandle, pSilentAimValues, 0, MEM_RELEASE);
        pSilentAimHook = nullptr;
        pSilentAimValues = nullptr;
        pSilentHookInitialized = false;
    }
}

void caimbot::think() {
    if (!NtUserSendInput) {
        NtUserSendInput = (NtUserSendInput_t)GetProcAddress(GetModuleHandleA("win32u"), "NtUserSendInput");
    }
    if (config.Mode == AimbotMode::None) {
        CleanupSilentHook();
        return;
    }

    bool keyPressed = GetAsyncKeyState(config.activationKey) & 0x8000;
    Vector_t eyePos, angles;
    Vector_t punchAngle, punchAngleVel;
    int shotsFired;
    int punchTickBase;
    float punchTickFraction;
    uintptr_t punchCacheAddr;
    int punchCacheCount;

    uintptr_t localPawn = entitysystem.CLocalPlayer.Address;

    if (config.recoilControlSystem) {
        shotsFired = memory.Read<int>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);
        punchAngle = memory.Read<Vector_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);
        punchAngleVel = memory.Read<Vector_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngleVel);
        punchTickBase = memory.Read<int>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchTickBase);
        punchTickFraction = memory.Read<float>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchTickFraction);
        punchCacheAddr = memory.Read<uintptr_t>(localPawn + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache);
        punchCacheCount = memory.Read<int>(punchCacheAddr + 0x8); 
    }

    if (config.Mode == AimbotMode::Silent || config.Mode == AimbotMode::Memory || config.Mode == AimbotMode::Mouse) {
        eyePos = memory.Read<Vector_t>(entitysystem.CLocalPlayer.Address + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_vecLastClipCameraPos);
        angles = GetViewAngels();
    }

    if (config.Mode != AimbotMode::Silent && pSilentHookInitialized) {
        CleanupSilentHook();
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

        uintptr_t boneArray = memory.Read<uintptr_t>(entity.CGameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
        Vector_t headPos = memory.Read<Vector_t>(boneArray + 6 * 32);

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
        if (config.Mode == AimbotMode::Silent || config.Mode == AimbotMode::Memory) {
            Vector_t delta = bestTargetPos - eyePos;
            float dist = delta.Length2D();
            float yaw = atan2f(delta.y, delta.x) * 57.295779513f;
            float pitch = -atan2f(delta.z, dist) * 57.295779513f;

            if (config.recoilControlSystem && shotsFired > 1) {
                float rcsYaw = punchAngle.y * config.recoilControlSystemX;
                float rcsPitch = punchAngle.x * config.recoilControlSystemY;

                rcsYaw += punchAngleVel.y * punchTickFraction * 0.1f;
                rcsPitch += punchAngleVel.x * punchTickFraction * 0.1f;

                if (punchCacheCount > 0 && punchCacheCount <= 0xFFFF) {
                    Vector_t lastPunch = memory.Read<Vector_t>(punchCacheAddr + 0x10 + (punchCacheCount - 1) * sizeof(Vector_t));
                    rcsYaw = (rcsYaw + lastPunch.y * config.recoilControlSystemX) * 0.5f;  
                    rcsPitch = (rcsPitch + lastPunch.x * config.recoilControlSystemY) * 0.5f;
                }

                yaw -= rcsYaw;
                pitch -= rcsPitch;
            }

            float yawDiff = yaw - angles.y;
            float pitchDiff = pitch - angles.x;

            while (yawDiff > 180.0f) yawDiff -= 360.0f;
            while (yawDiff < -180.0f) yawDiff += 360.0f;
            while (pitchDiff > 180.0f) pitchDiff -= 360.0f;
            while (pitchDiff < -180.0f) pitchDiff += 360.0f;

            float newYaw = angles.y + yawDiff;
            float newPitch = angles.x + pitchDiff;

            if (newPitch > 89.0f) newPitch = 89.0f;
            if (newPitch < -89.0f) newPitch = -89.0f;

            SetCameraPos({ newPitch, newYaw, 0.0f });
        }
        else if (config.Mode == AimbotMode::Mouse) {
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
                        Vector_t lastPunch = memory.Read<Vector_t>(punchCacheAddr + 0x10 + (punchCacheCount - 1) * sizeof(Vector_t));
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
    else if (config.Mode == AimbotMode::Silent) {
        SetCameraPos(GetViewAngels());
    }

    if (config.Mode == AimbotMode::Silent && !pSilentHookInitialized) {
        pSilentAimHook = VirtualAllocEx(memory.processHandle, NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        pSilentAimValues = VirtualAllocEx(memory.processHandle, NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

        if (!pSilentAimHook || !pSilentAimValues) {
            return;
        }

        BYTE jmp_buffer[sizeof(hk_jmp)];
        memcpy(jmp_buffer, hk_jmp, sizeof(hk_jmp));
        memcpy(jmp_buffer + 2, &pSilentAimHook, sizeof(uintptr_t));
        memory.WriteRaw(vars::module_client + SILENT_JMP, jmp_buffer, sizeof(jmp_buffer));

        BYTE hk_buffer[sizeof(hk)];
        memcpy(hk_buffer, hk, sizeof(hk));
        memcpy(hk_buffer + 2, &pSilentAimValues, sizeof(uintptr_t));
        LPVOID returnAddr = (LPVOID)(vars::module_client + SILENT_RETURN);
        memcpy(hk_buffer + 21, &returnAddr, sizeof(uintptr_t));
        memory.WriteRaw((uintptr_t)pSilentAimHook, hk_buffer, sizeof(hk_buffer));

        pSilentHookInitialized = true;
    }
}


void ctriggerbot::think()
{
    if (!config.TriggerBot) return; 

    int m_iIDEntIndex = memory.Read<int>(entitysystem.CLocalPlayer.Address +
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