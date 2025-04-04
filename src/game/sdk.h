#pragma once
#ifndef SDK_HPP
#define SDK_HPP

#include "../../libarys/memory/memory.hpp"
#include "../overlay/overlay.h"
#include "vars.h"
#include "offsets.hpp"
#include "vector.hpp"

#include <cstdint>
#include <stdio.h>
#include <Windows.h>
#include <chrono>
#include <string>
#include <array>
#include <future>


inline Vector_t world_to_screen(const Vector_t& pos) {
	if (!vars::view_matrix[0] || !vars::view_matrix[1] || !vars::view_matrix[3]) {
		return { 0, 0, 0 };
	}

	float _x = vars::view_matrix[0][0] * pos.x + vars::view_matrix[0][1] * pos.y + vars::view_matrix[0][2] * pos.z + vars::view_matrix[0][3];
	float _y = vars::view_matrix[1][0] * pos.x + vars::view_matrix[1][1] * pos.y + vars::view_matrix[1][2] * pos.z + vars::view_matrix[1][3];
	float w = vars::view_matrix[3][0] * pos.x + vars::view_matrix[3][1] * pos.y + vars::view_matrix[3][2] * pos.z + vars::view_matrix[3][3];

	if (w < 0.01f) {
		return { 0, 0, 0 };
	}

	float inv_w = w > 0.001f ? 1.f / w : 1.f;
	_x *= inv_w;
	_y *= inv_w;

	float screen_x = overlay::G_Width * 0.5f;
	float screen_y = overlay::G_Height * 0.5f;

	screen_x += 0.5f * _x * overlay::G_Width + 0.5f;
	screen_y -= 0.5f * _y * overlay::G_Height + 0.5f;

	return { screen_x, screen_y, w };
}

inline class C_IGlobalVars {
public:
	float m_flRealTime;
	int32_t m_nFrameCount;
	float m_flFrameTime;
	float m_flFrameTime2;
	int32_t m_nMaxClients;
private:
	char pad0x1c[0x1C];
public:
	float m_flIntervalPerSubTick;
	float m_flCurrentTime;
	float m_flCurrentTime2;
private:
	char pad0x14[0x14];
public:
	int32_t m_nTickCount;
} IGlobalVars;

inline class c_csgo {
public:
	std::uintptr_t csgo_client();
	std::uintptr_t csgo_engine2();
	std::uintptr_t csgo_entitylist();
	int csgo_buildnumber();
	cs_window csgo_window();
	int csgo_highestentity();
	void updateIGlobalVars();
	bool csgo_update();
	void csgo_force_cache();
} csgo;

class C_CEntityIdentity {
public:
	uintptr_t Address = 0;
	std::string m_designerName = "";

	bool get_m_designerName() {
		if (!Address) return false;

		uintptr_t DesignerNameAddress = memory.Read<uintptr_t>(Address + cs2_dumper::schemas::client_dll::CEntityIdentity::m_designerName);
		if (!DesignerNameAddress) return false;

		char NameBuffer[MAX_PATH] = {};
		if (!memory.ReadRaw(DesignerNameAddress, &NameBuffer, sizeof(NameBuffer))) {
			return false;
		}

		this->m_designerName = std::string(NameBuffer);
		return true;
	}

	bool Update() {
		if (!Address) return false;
		return get_m_designerName();
	}
};

class C_CEntityInstance {
public:
	uintptr_t Address = 0;
	C_CEntityIdentity CEntityIdentity;

	bool Update() {
		if (!Address) return false;

		CEntityIdentity.Address = memory.Read<uintptr_t>(Address + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity);
		if (!CEntityIdentity.Address) return false;

		return CEntityIdentity.Update();
	}
};

static C_CEntityInstance GetEntity(int nIdx) noexcept {
	C_CEntityInstance entity;
	uintptr_t uListEntry = memory.Read<uintptr_t>(vars::cs2_entitylist + 8LL * ((nIdx & 0x7FFF) >> 9) + 16);
	if (!uListEntry) return entity;

	entity.Address = memory.Read<uintptr_t>(uListEntry + 120LL * (nIdx & 0x1FF));
	return entity;
}

static uintptr_t GetEntityAddress(uintptr_t address) noexcept {
	uintptr_t uListEntry = memory.Read<uintptr_t>(vars::cs2_entitylist + 0x8 * ((address & 0x7FFF) >> 9) + 16);
	if (!uListEntry) return 0;

	return memory.Read<uintptr_t>(uListEntry + 120LL * (address & 0x1FF));
}

class C_BaseEntity : public C_CEntityInstance {
public:
	uintptr_t CGameSceneNode = 0;

	int m_iHealth = 0;
	int m_lifeState = 0;
	bool m_bTakesDamage = 0;
	float m_flSpeed = 0;
	int m_iTeamNum = 0;
	int m_spawnflags = 0;
	int m_fFlags = 0;
	Vector_t m_vecAbsVelocity{};

	bool IsDead() const {
		return m_iHealth == 0;
	}

	bool Update() {
		if (!Address) return false;

		this->CGameSceneNode = memory.Read<uintptr_t>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pGameSceneNode);
		if (!this->CGameSceneNode) return false;

		this->m_iHealth = memory.Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
		this->m_lifeState = memory.Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_lifeState);
		this->m_bTakesDamage = memory.Read<bool>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_bTakesDamage);
		this->m_flSpeed = memory.Read<float>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_flSpeed);
		this->m_iTeamNum = memory.Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
		this->m_spawnflags = memory.Read<int>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_spawnflags);
		this->m_vecAbsVelocity = memory.Read<Vector_t>(this->Address + cs2_dumper::schemas::client_dll::C_BaseEntity::m_vecAbsVelocity);

		return true;
	}

	Vector_t GetBasePosition() const {
		if (!CGameSceneNode) return Vector_t{ 0, 0, 0 };
		return memory.Read<Vector_t>(CGameSceneNode + cs2_dumper::schemas::client_dll::CGameSceneNode::m_vecAbsOrigin);
	}

};

constexpr std::uint32_t MAX_ENTITIES_IN_LIST = 512;
constexpr std::uint32_t MAX_ENTITY_LISTS = 64;
constexpr std::uint32_t MAX_TOTAL_ENTITIES = MAX_ENTITIES_IN_LIST * MAX_ENTITY_LISTS;

inline class c_entitysystem {
public:
	std::vector<C_CEntityInstance> CEntityInstances;
	std::vector<C_BaseEntity> CBasePlayerEntities;
	std::vector<std::string> Spectators;

	C_BaseEntity CLocalPlayer;

	std::chrono::steady_clock::time_point LastLoopTime = std::chrono::steady_clock::now();
	int CEntityInstancesCount = 0;

	void think() {
		auto CurrentTime = std::chrono::steady_clock::now();

		if (std::chrono::duration_cast<std::chrono::seconds>(CurrentTime - LastLoopTime).count() >= 4) {
			LastLoopTime = CurrentTime;
			CEntityInstancesCount = 0;

			csgo.csgo_update();

			CLocalPlayer.Address = memory.Read<uintptr_t>(vars::module_client + cs2_dumper::offsets::client_dll::dwLocalPlayerPawn);
			if (!CLocalPlayer.Update()) {
				CLocalPlayer.Address = 0;
			}

			Spectators.clear();

			uintptr_t CSLocalPlayerPawn = CLocalPlayer.Address;

			CEntityInstances.resize(MAX_TOTAL_ENTITIES + 1);
			CBasePlayerEntities.clear();
			for (int idx = 0; idx < MAX_TOTAL_ENTITIES; idx++) {
				C_CEntityInstance temp = GetEntity(idx);
				if (temp.Address == 0) {
					CEntityInstances[idx].Address = 0;
					continue;
				}

				CEntityInstances[idx].Address = temp.Address;
				if (!CEntityInstances[idx].Update()) {
					CEntityInstances[idx].Address = 0;
					continue;
				}

				if (CEntityInstances[idx].CEntityIdentity.m_designerName == "cs_player_controller") {
					uintptr_t m_hPawn = GetEntityAddress(memory.Read<uintptr_t>(CEntityInstances[idx].Address + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn));
					if (!m_hPawn) continue;

					C_BaseEntity BaseEntity{ m_hPawn };
					if (!BaseEntity.Update()) continue;

					uintptr_t observerServices = memory.Read<uintptr_t>(m_hPawn + cs2_dumper::schemas::client_dll::C_BasePlayerPawn::m_pObserverServices);
					if (observerServices) {
						uint64_t observerTargetHandle = memory.Read<uint64_t>(observerServices + cs2_dumper::schemas::client_dll::CPlayer_ObserverServices::m_hObserverTarget);
						uintptr_t observerTarget = GetEntityAddress(observerTargetHandle);

						if (observerTarget == CSLocalPlayerPawn && observerTarget != 0) {
							std::string spectatorName = "Unknown";
							uintptr_t controllerNameAddr = memory.Read<uintptr_t>(CEntityInstances[idx].Address + cs2_dumper::schemas::client_dll::CCSPlayerController::m_sSanitizedPlayerName);
							if (controllerNameAddr) {
								char nameBuffer[MAX_PATH] = {};
								if (memory.ReadRaw(controllerNameAddr, &nameBuffer, sizeof(nameBuffer))) {
									spectatorName = std::string(nameBuffer);
								}
							}
							Spectators.push_back(spectatorName);
						}
					}

					CBasePlayerEntities.push_back(std::move(BaseEntity));
				}
				CEntityInstancesCount++;
			}
		}
	}
} entitysystem;

#endif