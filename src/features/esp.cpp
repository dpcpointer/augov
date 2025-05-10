#include "esp.h"

enum BONEINDEX : DWORD
{
	pelvis = 0,
	spine_0 = 1,
	spine_1 = 2,
	spine_2 = 3,
	spine_3 = 4,
	neck_0 = 5,
	head_0 = 6,
	clavicle_l = 7,
	arm_upper_l = 8,
	arm_lower_l = 9,
	hand_l = 10,
	weapon_hand_l = 11,
	clavicle_r = 12,
	arm_upper_r = 13,
	arm_lower_r = 14,
	hand_r = 15,
	weapon_hand_r = 16,
	jiggle_primary = 17,
	chesthier_offset = 18,
	weaponhier_jnt = 19,
	weaponhier_r_iktarget = 20,
	weaponhier_l_iktarget = 21,
	leg_upper_l = 22,
	leg_lower_l = 23,
	ankle_l = 24,
	leg_upper_r = 25,
	leg_lower_r = 26,
	ankle_r = 27,
	root_motion = 28,
	leg_l_offset = 29,
	leg_l_iktarget = 30,
	leg_r_offset = 31,
	leg_r_iktarget = 32,
	eyeball_l = 33,
	eyeball_r = 34,
	eye_target = 35,
	head_0_twist = 36,
	arm_lower_l_twist = 37,
	arm_lower_l_twist1 = 38,
	finger_middle_meta_l = 39,
	finger_middle_0_l = 40,
	finger_middle_1_l = 41,
	finger_middle_2_l = 42,
	finger_pinky_meta_l = 43,
	finger_pinky_0_l = 44,
	finger_pinky_1_l = 45,
	finger_pinky_2_l = 46,
	finger_index_meta_l = 47,
	finger_index_0_l = 48,
	finger_index_1_l = 49,
	finger_index_2_l = 50,
	finger_thumb_0_l = 51,
	finger_thumb_1_l = 52,
	finger_thumb_2_l = 53,
	finger_ring_meta_l = 54,
	finger_ring_0_l = 55,
	finger_ring_1_l = 56,
	finger_ring_2_l = 57,
	arm_upper_l_twist1 = 58,
	arm_upper_l_twist = 59,
	pect_l_aimat = 60,
	scapula_l = 61,
	arm_lower_r_twist = 62,
	arm_lower_r_twist1 = 63,
	finger_middle_meta_r = 64,
	finger_middle_0_r = 65,
	finger_middle_1_r = 66,
	finger_middle_2_r = 67,
	finger_pinky_meta_r = 68,
	finger_pinky_0_r = 69,
	finger_pinky_1_r = 70,
	finger_pinky_2_r = 71,
	finger_index_meta_r = 72,
	finger_index_0_r = 73,
	finger_index_1_r = 74,
	finger_index_2_r = 75,
	finger_thumb_0_r = 76,
	finger_thumb_1_r = 77,
	finger_thumb_2_r = 78,
	finger_ring_meta_r = 79,
	finger_ring_0_r = 80,
	finger_ring_1_r = 81,
	finger_ring_2_r = 82,
	arm_upper_r_twist1 = 83,
	arm_upper_r_twist = 84,
	pect_r_aimat = 85,
	scapula_r = 86,
	pect_l_aimup = 87,
	pect_r_aimup = 88,
	scap_aimup = 89,
	pectaim_l = 90,
	pecttrans_l = 91,
	pectaim_r = 92,
	pecttrans_r = 93,
	scap_r_aimat = 94,
	scap_l_aimat = 95,
	pect_l_ptbase = 96,
	pect_r_ptbase = 97,
	ball_l = 98,
	leg_upper_l_twist = 99,
	leg_upper_l_twist1 = 100,
	ball_r = 101,
	leg_upper_r_twist = 102,
	leg_upper_r_twist1 = 103,
	feet_l = 104,
	feet_r = 109,
	knife_attachment = 114,
	main_weapon_attachment = 124,
	pistol_attachment = 113

};

void cesp::renderBoxHealth(const C_BaseEntity& CBaseEntity, bool renderText)
{
	Vector_t basePosition = CBaseEntity.GetBasePosition();
	Vector_t headPosition = { basePosition.x, basePosition.y, basePosition.z + 72.5f };

	Vector_t w2sHead = world_to_screen(headPosition);
	Vector_t w2sBase = world_to_screen(basePosition);

	if (w2sHead.IsZero() || w2sBase.IsZero()) return;

	Vector2D_t base = w2sBase.ToVector2D();
	Vector2D_t top = w2sHead.ToVector2D();

	float height = std::abs(base.y - top.y);
	float width = height * 0.55f;
	float barWidth = 2.f;

	float barX = top.x - width / 2 - barWidth;
	float barHeight = height * (CBaseEntity.m_iHealth / 100.0f);
	float barTopY = base.y - barHeight;

	ImVec2 barTopLeft = ImVec2(barX, barTopY);
	ImVec2 barBottomRight = ImVec2(barX + barWidth, base.y);

	ImGui::GetBackgroundDrawList()->AddRect(
		{ barX, top.y },
		{ barX + barWidth + 1, base.y + 1 },
		IM_COL32(0, 0, 0, 255),
		0.0f,
		0,
		1.5f
	);

	ImU32 healthColor;
	if (CBaseEntity.m_iHealth >= 100)
		healthColor = IM_COL32(0, 255, 0, 255);
	else if (CBaseEntity.m_iHealth >= 50)
		healthColor = IM_COL32(255, 255, 0, 255);
	else if (CBaseEntity.m_iHealth >= 25)
		healthColor = IM_COL32(255, 165, 0, 255);
	else
		healthColor = IM_COL32(255, 0, 0, 255);

	ImGui::GetBackgroundDrawList()->AddRectFilled(
		barTopLeft,
		barBottomRight,
		healthColor
	);

	if (renderText) {
		char healthText[20];
		sprintf_s(healthText, "%d", CBaseEntity.m_iHealth);
		ImVec2 textSize = ImGui::CalcTextSize(healthText);
		ImVec2 textPos = ImVec2(
			barX + (barWidth - textSize.x) / 2,
			barTopY - textSize.y / 2 + 2.5f
		);

		ImGui::GetBackgroundDrawList()->AddText(
			{ textPos.x + 1, textPos.y + 1 },
			IM_COL32(0, 0, 0, 200),
			healthText
		);
		ImGui::GetBackgroundDrawList()->AddText(
			{ textPos.x - 1, textPos.y - 1 },
			IM_COL32(0, 0, 0, 200),
			healthText
		);
		ImGui::GetBackgroundDrawList()->AddText(
			textPos,
			IM_COL32(255, 255, 255, 255),
			healthText
		);
	}
}

void cesp::renderSkeletonEsp(const C_BaseEntity& CBaseEntity) {
	struct BoneJointData {
		Vector_t Position;
		char pad[20];
	};
	const int maxBones = 128;
	BoneJointData boneData[maxBones];

	uintptr_t boneArray = m.read<uintptr_t>(CBaseEntity.CGameSceneNode + cs2_dumper::schemas::client_dll::CSkeletonInstance::m_modelState + 0x80);
	if (!boneArray) return;
	m.read(boneArray, boneData, sizeof(BoneJointData) * maxBones);

	struct BoneConnection {
		int from;
		int to;
	};

	const BoneConnection skeleton[] = {
		{head_0, neck_0}, {neck_0, spine_3}, {spine_3, spine_2}, {spine_2, spine_1},
		{spine_1, spine_0}, {spine_0, pelvis},
		{head_0, eyeball_l}, {head_0, eyeball_r},
		{spine_3, clavicle_l}, {clavicle_l, arm_upper_l}, {arm_upper_l, arm_lower_l}, {arm_lower_l, hand_l},
		{spine_3, clavicle_r}, {clavicle_r, arm_upper_r}, {arm_upper_r, arm_lower_r}, {arm_lower_r, hand_r},
		{pelvis, leg_upper_l}, {leg_upper_l, leg_lower_l}, {leg_lower_l, ankle_l},
		{pelvis, leg_upper_r}, {leg_upper_r, leg_lower_r}, {leg_lower_r, ankle_r}
	};

	Vector_t BaseEntityPos = CBaseEntity.GetBasePosition();
	constexpr float TARGET_DISTANCE = 75.0f;
	constexpr float DISTANCE_TOLERANCE = 75.f; 
	constexpr float MIN_DISTANCE = TARGET_DISTANCE - DISTANCE_TOLERANCE;
	constexpr float MAX_DISTANCE = TARGET_DISTANCE + DISTANCE_TOLERANCE;

	constexpr int numConnections = sizeof(skeleton) / sizeof(skeleton[0]);
	for (int i = 0; i < numConnections; i++) {
		const BoneConnection& conn = skeleton[i];

		Vector_t fromWorldPos = boneData[conn.from].Position;
		Vector_t toWorldPos = boneData[conn.to].Position;

		if (fromWorldPos.IsZero() || toWorldPos.IsZero())
			continue;

		float fromDistance = fromWorldPos.DistTo(BaseEntityPos);
		float toDistance = toWorldPos.DistTo(BaseEntityPos);

		if (fromDistance > MAX_DISTANCE || toDistance > MAX_DISTANCE)
			continue;

		float boneDistance = fromWorldPos.DistTo(toWorldPos);

		if (boneDistance < MIN_DISTANCE || boneDistance > MAX_DISTANCE)
			continue;

		Vector_t fromPos = world_to_screen(fromWorldPos);
		Vector_t toPos = world_to_screen(toWorldPos);

		if (fromPos.IsZero() || toPos.IsZero())
			continue;

		ImGui::GetBackgroundDrawList()->AddLine(
			ImVec2(fromPos.x, fromPos.y),
			ImVec2(toPos.x, toPos.y),
			IM_COL32(255, 255, 255, 255),
			1.0f
		);
	}
}

void cesp::renderBoxEsp(const C_BaseEntity& CBaseEntity)
{
	Vector_t basePosition = m.read<Vector_t>(CBaseEntity.Address + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_vOldOrigin);
	Vector_t headPosition = { basePosition.x, basePosition.y, basePosition.z + 72.5f };

	Vector_t w2sHead = world_to_screen(headPosition);
	Vector_t w2sBase = world_to_screen(basePosition);

	if (w2sHead.IsZero() || w2sBase.IsZero()) return;

	Vector2D_t base = w2sBase.ToVector2D();
	Vector2D_t top = w2sHead.ToVector2D();

	float height = base.y - top.y;
	float width = height * 0.55f;

	ImGui::GetBackgroundDrawList()->AddRect(
		{ top.x - width / 2 + 1.f , top.y   - 1.f},
		{ top.x + width / 2 + 1.f  , base.y - 1.f },
		IM_COL32(1, 1, 1, 255)
	);

	ImGui::GetBackgroundDrawList()->AddRect(
		{ top.x - width / 2, top.y },
		{ top.x + width / 2, base.y },
		IM_COL32(225, 255, 255, 155)
	);
}

void cesp::renderBoxEsp(const C_BaseEntity& CBaseEntity, const BoxEspParams& boxParam)
{
	float height = boxParam.base.y - boxParam.top.y;
	float width = height * boxParam.width;

	ImGui::GetBackgroundDrawList()->AddRect(
		{ boxParam.top.x - width / 2 + 1.f ,  boxParam.top.y - 1.f },
		{ boxParam.top.x + width / 2 + 1.f  , boxParam.base.y - 1.f },
		IM_COL32(1, 1, 1, 255)
	);

	ImGui::GetBackgroundDrawList()->AddRect(
		{ boxParam.top.x - width / 2, boxParam.top.y },
		{ boxParam.top.x + width / 2, boxParam.base.y },
		IM_COL32(225, 255, 255, 155)
	);
}

void cesp::think() {
	vars::view_matrix = m.read<view_matrix_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwViewMatrix);

	for (auto& BaseEntity : entitysystem.CBasePlayerEntities) {
		if (!BaseEntity.Update()) continue;
		if (BaseEntity.IsDead() || BaseEntity.m_iTeamNum == entitysystem.CLocalPlayer.m_iTeamNum) continue;
		

		float BaseEntityDistance;
		Vector_t BaseEntityPos = BaseEntity.GetBasePosition();
		Vector_t LocalBaseEntityPos = entitysystem.CLocalPlayer.GetBasePosition();
		BaseEntityDistance = BaseEntityPos.DistTo(LocalBaseEntityPos) / 12.5f;

		int fontIndex;
		if (BaseEntityDistance < 10.0f) fontIndex = 5;
		else if (BaseEntityDistance < 20.0f) fontIndex = 4;
		else if (BaseEntityDistance < 30.0f) fontIndex = 3;
		else if (BaseEntityDistance < 40.0f) fontIndex = 2;
		else if (BaseEntityDistance < 50.0f) fontIndex = 1;
		else fontIndex = 0;
		ImGui::PushFont(FontScallingArray[fontIndex]);

		if (config.cs_player_controller_esp) {
			renderBoxEsp(BaseEntity);
			if (config.cs_player_controller_skeleton)
			{
				renderSkeletonEsp(BaseEntity);
			}

			if (config.cs_player_controller_health) {
				renderBoxHealth(BaseEntity, config.cs_player_controller_healthtext);
			}
		}

		ImGui::PopFont();
	}

	for (const auto& CEntityInstance : entitysystem.CEntityInstances) {
		if (CEntityInstance.CEntityIdentity.m_designerName == "chicken") {
			C_BaseEntity BaseEntity{ CEntityInstance.Address };
			if (!BaseEntity.Update()) continue;

			Vector_t basePosition = BaseEntity.GetBasePosition();
			Vector_t topPosition = { basePosition.x, basePosition.y, basePosition.z + 20.f };

			Vector_t w2sTop = world_to_screen(topPosition);
			Vector_t w2sBase = world_to_screen(basePosition);

			if (w2sTop.IsZero() || w2sBase.IsZero()) continue;

			cesp::BoxEspParams BoxParams;
			BoxParams.base = w2sBase.ToVector2D();
			BoxParams.top = w2sTop.ToVector2D();
			BoxParams.width = 1.2f;

			if (config.chicken_esp) {
				renderBoxEsp(BaseEntity, BoxParams);
			}
		}

		if (CEntityInstance.CEntityIdentity.m_designerName == "smokegrenade_projectile") {
			C_BaseEntity BaseEntity{ CEntityInstance.Address };
			if (!BaseEntity.Update()) continue;

			Vector_t basePosition = BaseEntity.GetBasePosition();
			Vector_t topPosition = { basePosition.x, basePosition.y, basePosition.z + 20.f };

			Vector_t w2sTop = world_to_screen(topPosition);
			Vector_t w2sBase = world_to_screen(basePosition);

			if (w2sTop.IsZero() || w2sBase.IsZero()) continue;

			ImGui::GetBackgroundDrawList()->AddText(
				{ w2sBase.x, w2sBase.y },
				ImColor(255, 255, 255, 255),
				CEntityInstance.CEntityIdentity.m_designerName.c_str()
			);
		}
	}
}