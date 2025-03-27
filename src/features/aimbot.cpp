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
        input.mi.dx = static_cast<LONG>(Position.x);
        input.mi.dy = static_cast<LONG>(Position.y);
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

        if (distanceToCenter <= fovRadius) {
            bestTargetPos = headPos;
            found = true;
            break;
        }
    }

    if (found && keyPressed) {
        if (config.Mode == AimbotMode::Silent || config.Mode == AimbotMode::Memory) {
            Vector_t delta = bestTargetPos - eyePos;
            float dist = delta.Length2D();
            float yaw = atan2f(delta.y, delta.x) * 57.295779513f;
            float pitch = -atan2f(delta.z, dist) * 57.295779513f;

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