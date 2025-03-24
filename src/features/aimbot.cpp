#include "aimbot.h"

Vector_t caimbot::GetViewAngels() {
    return memory.Read<Vector_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewAngles);
}

void caimbot::SetCameraPos(Vector_t Position) {
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
    if (config.Mode == AimbotMode::None) {
        CleanupSilentHook();
        return;
    }

    bool keyPressed = GetAsyncKeyState(config.activationKey) & 0x8000;
    Vector_t eyePos, angles, targetPos;

    if (config.Mode == AimbotMode::Silent || config.Mode == AimbotMode::Memory) {
        eyePos = memory.Read<Vector_t>(entitysystem.CLocalPlayer.Address + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_vecLastClipCameraPos);
        angles = memory.Read<Vector_t>(entitysystem.CLocalPlayer.Address + cs2_dumper::schemas::client_dll::C_CSPlayerPawnBase::m_angEyeAngles);
    }

    if (config.Mode != AimbotMode::Silent && pSilentHookInitialized) {
        CleanupSilentHook();
    }

    float closest = FLT_MAX;
    bool found = false;
    for (auto& entity : entitysystem.CBasePlayerEntities) {
        if (!entity.Address || entity.IsDead() || entity.m_iTeamNum == entitysystem.CLocalPlayer.m_iTeamNum) continue;
        uintptr_t boneArray = memory.Read<uintptr_t>(entity.CGameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
        Vector_t pos = memory.Read<Vector_t>(boneArray + 6 * 32);
        float dist = eyePos.DistTo(pos);
        if (dist < closest) {
            closest = dist;
            targetPos = pos;
            found = true;
        }
    }

    if (config.Mode == AimbotMode::Silent) {
        if (!pSilentHookInitialized) {
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

        if (!found) {
            SetCameraPos(GetViewAngels());
        }
        else if (keyPressed) {
            Vector_t delta = targetPos - eyePos;
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
        else {
            SetCameraPos(GetViewAngels());
        }
    }
    if (config.Mode == AimbotMode::Memory)
    {
        if (keyPressed) {
            Vector_t delta = targetPos - eyePos;
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
    }
}