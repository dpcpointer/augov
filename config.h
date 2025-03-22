#pragma once
#ifndef CONFIG_H
#define CONFIG_



inline class cconfig
{
public:
	bool cs_player_controller_esp;
	bool chicken_esp;

	bool aimbot = false;

	enum class AimbotMode : int
	{
		Silent,
		Memory,
		Mouse
	};

	AimbotMode Mode = AimbotMode::Mouse;
	float fieldOfView = 90.0f;
	float smoothness = 1.0f;
	bool recoilControl = false;
	int activationKey = 0;





}config;

#endif // !CONFIG_H
