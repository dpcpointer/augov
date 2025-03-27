#pragma once
#ifndef CONFIG_H
#define CONFIG_


enum class AimbotMode : int
{
	Silent,
	Memory,
	Mouse, 
	None
};

inline class cconfig
{
public:
	bool cs_player_controller_esp = false;
	bool chicken_esp = false;

	bool cs_player_controller_health = false;
	bool cs_player_controller_healthtext = false;
	bool cs_player_controller_skeleton = false;


	bool aimbot = true;

	AimbotMode Mode = AimbotMode::None;
	float fieldOfView = 90.0f;
	float smoothness = 1.0f;
	int activationKey = 0x06;
	bool recoilControlSystem;
	float recoilControlSystemX;
	float recoilControlSystemY;

	int triggeractivationKey = 0x06;
	bool TriggerBot;





} config;

#endif // !CONFIG_H
