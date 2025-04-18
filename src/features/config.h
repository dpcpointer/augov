#pragma once
#ifndef CONFIG_H
#define CONFIG_

#include <Windows.h>

enum class AimbotMode : int
{
	Mouse, 
	None
};

enum class AimbotKey : int
{
	NONE = 0,
	MOUSE_LEFT = VK_LBUTTON,    // 0x01
	MOUSE_RIGHT = VK_RBUTTON,   // 0x02
	MOUSE_X1 = VK_XBUTTON1,     // 0x05 (Side button 1)
	MOUSE_X2 = VK_XBUTTON2,     // 0x06 (Side button 2)
	SHIFT = VK_SHIFT,
	CTRL = VK_CONTROL,
	ALT = VK_MENU,
	SPACE = VK_SPACE,
	Q = 0x51,
	E = 0x45,
	F = 0x46
};
inline bool IsKeyPressed(AimbotKey key) {
	if (key == AimbotKey::NONE) return false;
	return GetAsyncKeyState(static_cast<int>(key)) & 0x8000;
}

inline class cconfig
{
public:
	bool cs_player_controller_esp = false;
	bool chicken_esp = false;

	bool cs_player_controller_health = false;
	bool cs_player_controller_healthtext = false;
	bool cs_player_controller_skeleton = false;
	bool spectatoresp; 

	bool aimbot = true;

	AimbotMode Mode = AimbotMode::None;
	float fieldOfView = 90.0f;
	float smoothness = 1.0f;
	AimbotKey activationKey = AimbotKey::NONE;
	bool recoilControlSystem;
	float recoilControlSystemX;
	float recoilControlSystemY;

	int triggeractivationKey = 0x06;
	bool TriggerBot;
	
	float aimAmt;
	bool bhop = false;




} config;

#endif // !CONFIG_H
