#pragma once
#ifndef ESP_H
#define ESP_H

#include "../game/sdk.h"
#include "config.h"
#include "../overlay/overlay.h"


inline class cesp
{
public:
	ImFont* FontScallingArray[6];
	struct BoxEspParams
	{
		Vector2D_t base;
		Vector2D_t top;
		float width = 0;
	};

	void renderSkeletonEsp(const C_BaseEntity& CBaseEntity);
	void renderBoxEsp(const C_BaseEntity& CBaseEntity);
	void renderBoxHealth(const C_BaseEntity& CBaseEntity, bool renderText);
	void renderBoxEsp(const C_BaseEntity& CBaseEntity, const BoxEspParams& boxParam);

	void think();
} esp;

#endif // !ESP_H