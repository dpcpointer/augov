#pragma once
#ifndef ESP_H
#define ESP_H

#include "sdk.h"
#include "config.h"
#include "Overlay.h"



inline class cesp
{
public:
	struct BoxEspParams
	{
		Vector2D_t base;
		Vector2D_t top;
		float width;
	};

	void renderBoxEsp(C_BaseEntity CBaseEntity);
	void renderBoxEsp(C_BaseEntity CBaseEntity, BoxEspParams& boxParam);

	void think();
}esp;

#endif // !ESP_H