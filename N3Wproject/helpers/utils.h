#pragma once

#define NOMINMAX
#include <Windows.h>
#include <string>
#include <initializer_list>
#include "../valve_sdk/math/QAngle.h"
#include "../valve_sdk/sdk.h"

namespace utils 
{
	unsigned int find_in_data_map(datamap_t * p_map, const char * name);
    int wait_for_modules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules);
    std::uint8_t* pattern_scan(void* module, const char* signature);

	void attach_console();
    bool console_print(const char* fmt, ...);
    HMODULE get_module(const std::string& name);
    std::uint8_t* pattern_scan(const char* moduleName, const char* signature);
    char console_read_key();
    void detach_console();
    bool is_play_mm();
    bool is_valve_ds();
    bool Insecure();
    bool message_send();
    unsigned int GetVirtual(void* class_, unsigned int index);
    bool is_ingame();
    int epoch_time();
    float corrected_tickbase(c_usercmd* cmd);
    float get_curtime(c_usercmd* ucmd);
	bool can_lag(const bool& voice_check = true);
    float lerp(float a, float b, float f);
	int random(const int& min, const int& max);
	float random(const float& min, const float& max);
	bool hitchance(c_base_entity* entity, const QAngle& angles, const float& chance, const float& hit_count = 150.f, const int& hit_group = -1);
    void set_clantag(std::string tag);
    bool set_name(bool reconnect, const char* newName, float delay);
    void rank_reveal_all();
    std::wstring s2ws(const std::string& str);
    std::string ws2s(const std::wstring& wstr);
}
