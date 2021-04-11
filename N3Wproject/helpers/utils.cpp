#include "Utils.h"
#include "../hooks/hooks.h"

#define NOMINMAX
#include <Windows.h>
#include <stdio.h>
#include <chrono>
#include <string>
#include <vector>
#include <Tlhelp32.h>
#include <codecvt>


#include "../valve_sdk/csgostructs.h"
#include "math.h"
#include "../hooks/importer.h"









HANDLE _out = NULL, _old_out = NULL;
HANDLE _err = NULL, _old_err = NULL;
HANDLE _in = NULL, _old_in = NULL;

namespace utils
{
    unsigned int find_in_data_map(datamap_t* p_map, const char* name)
    {
        while (p_map)
        {
            for (int i = 0; i < p_map->dataNumFields; i++)
            {
                if (p_map->dataDesc[i].fieldName == NULL)
                    continue;

                if (strcmp(name, p_map->dataDesc[i].fieldName) == 0)
                    return p_map->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

                if (p_map->dataDesc[i].fieldType == FIELD_EMBEDDED)
                {
                    if (p_map->dataDesc[i].td)
                    {
                        unsigned int offset;

                        if ((offset = find_in_data_map(p_map->dataDesc[i].td, name)) != 0)
                            return offset;
                    }
                }
            }
            p_map = p_map->baseMap;
        }

        return 0;
    }

    float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }

    bool is_ingame()
    {
        return g_engine_client->is_connected() && g_engine_client->is_ingame();
    }

    void attach_console()
    {
        _old_out = GetStdHandle(STD_OUTPUT_HANDLE);
        _old_err = GetStdHandle(STD_ERROR_HANDLE);
        _old_in  = GetStdHandle(STD_INPUT_HANDLE);

        ::AllocConsole() && ::AttachConsole(GetCurrentProcessId());

        _out     = GetStdHandle(STD_OUTPUT_HANDLE);
        _err     = GetStdHandle(STD_ERROR_HANDLE);
        _in      = GetStdHandle(STD_INPUT_HANDLE);

        SetConsoleMode(_out,
            ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

        SetConsoleMode(_in,
            ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS |
            ENABLE_PROCESSED_INPUT | ENABLE_QUICK_EDIT_MODE);
    }

    bool Insecure()
    {
        return std::strstr(GetCommandLineA(), "-insecure");
    }

    std::map<std::string, HMODULE> modules = {};


    HMODULE get_module(const std::string& name)
    {
        const auto module_name = name == "client.dll" ? "client.dll" : name;

        if (modules.count(module_name) == 0 || !modules[module_name])
            modules[module_name] = LI_FN(GetModuleHandleA).cached()(module_name.c_str());

        return modules[module_name];
    }

   /* std::uint8_t* pattern_scan(const char* moduleName, const char* signature)
    {
        return pattern_scan(get_module(moduleName), signature);
    }*/

    float corrected_tickbase(c_usercmd* cmd)
    {
        c_usercmd* last_ucmd = nullptr;
        int corrected_tickbase = 0;

        corrected_tickbase = (!last_ucmd || last_ucmd->hasbeenpredicted) ? (float)g_local_player->m_nTickBase() : corrected_tickbase++;
        last_ucmd = cmd;
        float corrected_curtime = corrected_tickbase * g_global_vars->interval_per_tick;
        return corrected_curtime;

    };

    unsigned int GetVirtual(void* class_, unsigned int index) {
        return (unsigned int)(*(int**)class_)[index];
    }



    bool is_play_mm()
    {
        convar* type = nullptr;
        convar* mode = nullptr;

        if (!mode)
            mode = g_cvar->find_var("game_mode");

        if (!type)
            type = g_cvar->find_var("game_type");

        if (type->get_int() == 0 && mode->get_int() == 0) //casual
            return false;

        if (type->get_int() == 1 && mode->get_int() == 1) //demolition
            return false;

        if (type->get_int() == 1 && mode->get_int() == 0) //arms race
            return false;

        if (type->get_int() == 1 && mode->get_int() == 2) //deathmatch
            return false;

        if (type->get_int() == 0 && mode->get_int() == 1) //competetive
            return true;

        if (type->get_int() == 0 && mode->get_int() == 2) //wingman
            return true;

        if (type->get_int() == 6 && mode->get_int() == 0) //dangerzone
            return true;

        if (type->get_int() == 6 && mode->get_int() == 0) //scrimmage
            return true;

        return false;
    }

    bool is_valve_ds()
    {
        if (g_gamerules->m_bIsValveDS())
            return true;

        if (!g_gamerules->m_bIsValveDS())
            return false;

        return false;
    }

    float get_curtime(c_usercmd* ucmd) 
    {

        if (!g_local_player)
            return 0;

        int g_tick = 0;
        c_usercmd* g_pLastCmd = nullptr;
        if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
            g_tick = (float)g_local_player->m_nTickBase();
        }
        else {
            ++g_tick;
        }
        g_pLastCmd = ucmd;
        float curtime = g_tick * g_global_vars->interval_per_tick;
        return curtime;
    }

	bool can_lag(const bool& voice_check)
	{
		if (g_global_vars->interval_per_tick * 0.9f < g_global_vars->absoluteframetime)
			return false;

		if (voice_check && g_engine_client->is_voice_recording())
			return false;

		auto* channel_info = g_engine_client->get_net_channel_info();
		if (channel_info && (channel_info->get_avg_loss(1) > 0.f || channel_info->get_avg_loss(0) > 0.f))
			return false;

		return true;
	}

	int random(const int& min, const int& max)
	{
		return rand() % (max - min + 1) + min;
	}
   
   

	float random(const float& min, const float& max)
	{
		return ((max - min) * ((float)rand() / RAND_MAX)) + min;
	}

	bool hitchance(c_base_entity* entity, const QAngle& angles, const float& chance, const float& hit_count, const int& hit_group)
	{
		if (!g_local_player)
			return false;

		auto weapon = g_local_player->m_hActiveWeapon();
		if (!weapon || !weapon->is_weapon())
			return false;

		Vector forward, right, up;
		Vector src = g_local_player->get_eye_pos();
		math::angle_vectors(angles, forward, right, up);

		int cHits = 0;
		int cNeededHits = static_cast<int>(hit_count * (chance / 100.f));

		weapon->update_accuracy_penalty();
		float weap_spread = weapon->get_spread();
		float weap_inaccuracy = weapon->get_inaccuracy();

		ray_t ray;
		trace_t tr;
		c_tracefilter_players_only_skip_one filter(g_local_player);

		for (int i = 0; i < hit_count; i++)
		{
			float a = random(0.f, 1.f);
			float b = random(0.f, 2.f * M_PI);
			float c = random(0.f, 1.f);
			float d = random(0.f, 2.f * M_PI);

			float inaccuracy = a * weap_inaccuracy;
			float spread = c * weap_spread;

			if (weapon->m_Item().m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			{
				a = 1.f - a * a;
				a = 1.f - c * c;
			}

			Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

			direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * direction.x);
			direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * direction.y);
			direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * direction.z);
			direction.Normalized();

			QAngle viewAnglesSpread;
			math::vector_angles(direction, viewAnglesSpread);
			viewAnglesSpread.Normalize();

			Vector viewForward;
			math::angle_vectors(viewAnglesSpread, viewForward);
			viewForward.NormalizeInPlace();

			viewForward = src + (viewForward * weapon->get_cs_weapondata()->flRange);

			ray.init(src, viewForward);
			g_engine_trace->trace_ray(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

			if (tr.hit_entity == entity && (hit_group == -1 || hit_group == tr.hitgroup))
				++cHits;

			const auto ch = static_cast<int>((static_cast<float>(cHits) / hit_count) * 100.f);
			if (ch >= chance)
				return true;

			if ((hit_count - i + cHits) < cNeededHits)
				return false;
		}

		return false;
	}

    void detach_console()
    {
        if(_out && _err && _in) {
            FreeConsole();

            if(_old_out)
                SetStdHandle(STD_OUTPUT_HANDLE, _old_out);
            if(_old_err)
                SetStdHandle(STD_ERROR_HANDLE, _old_err);
            if(_old_in)
                SetStdHandle(STD_INPUT_HANDLE, _old_in);
        }
    }

	int epoch_time()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

    bool console_print(const char* fmt, ...)
    {
        if(!_out) 
            return false;

        char buf[1024];
        va_list va;

        va_start(va, fmt);
        _vsnprintf_s(buf, 1024, fmt, va);
        va_end(va);

        return !!WriteConsoleA(_out, buf, static_cast<DWORD>(strlen(buf)), nullptr, nullptr);
    }

    /*
     * @brief Blocks execution until a key is pressed on the console window
     *
     */
    char console_read_key()
    {
        if(!_in)
            return false;

        auto key = char{ 0 };
        auto keysread = DWORD{ 0 };

        ReadConsoleA(_in, &key, 1, &keysread, nullptr);

        return key;
    }


    /*
     * @brief Wait for all the given modules to be loaded
     *
     * @param timeout How long to wait
     * @param modules List of modules to wait for
     *
     * @returns See WaitForSingleObject return values.
     */
    int wait_for_modules(std::int32_t timeout, const std::initializer_list<std::wstring>& modules)
    {
        bool signaled[32] = { 0 };
        bool success = false;

        std::uint32_t totalSlept = 0;

        if(timeout == 0)
        {
            for(auto& mod : modules) 
            {
                if(GetModuleHandleW(std::data(mod)) == NULL)
                    return WAIT_TIMEOUT;
            }
            return WAIT_OBJECT_0;
        }

        if(timeout < 0)
            timeout = INT32_MAX;

        while(true) {
            for(auto i = 0u; i < modules.size(); ++i)
            {
                auto& module = *(modules.begin() + i);
                if(!signaled[i] && GetModuleHandleW(std::data(module)) != NULL) 
                {
                    signaled[i] = true;

                    //
                    // Checks if all modules are signaled
                    //
                    bool done = true;
                    for(auto j = 0u; j < modules.size(); ++j) 
                    {
                        if(!signaled[j]) 
                        {
                            done = false;
                            break;
                        }
                    }
                    if(done) 
                    {
                        success = true;
                        goto exit;
                    }
                }
            }
            if(totalSlept > std::uint32_t(timeout)) 
            {
                break;
            }
            Sleep(10);
            totalSlept += 10;
        }

    exit:
        return success ? WAIT_OBJECT_0 : WAIT_TIMEOUT;
    }

    /*
     * @brief Scan for a given byte pattern on a module
     *
     * @param module    Base of the module to search
     * @param signature IDA-style byte array pattern
     *
     * @returns Address of the first occurence
     */
    std::uint8_t* pattern_scan(void* module, const char* signature)
    {
        static auto pattern_to_byte = [](const char* pattern)
        {
            auto bytes = std::vector<int>{};
            auto start = const_cast<char*>(pattern);
            auto end = const_cast<char*>(pattern) + strlen(pattern);

            for(auto current = start; current < end; ++current)
            {
                if(*current == '?') 
                {
                    ++current;
                    if(*current == '?')
                        ++current;
                    bytes.push_back(-1);
                } 
                else
                {
                    bytes.push_back(strtoul(current, &current, 16));
                }
            }
            return bytes;
        };

        auto dosHeader = (PIMAGE_DOS_HEADER)module;
        auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)module + dosHeader->e_lfanew);

        auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
        auto patternBytes = pattern_to_byte(signature);
        auto scanBytes = reinterpret_cast<std::uint8_t*>(module);

        auto s = patternBytes.size();
        auto d = patternBytes.data();

        for(auto i = 0ul; i < sizeOfImage - s; ++i) 
        {
            bool found = true;
            for(auto j = 0ul; j < s; ++j)
            {
                if(scanBytes[i + j] != d[j] && d[j] != -1) 
                {
                    found = false;
                    break;
                }
            }
            if(found) 
            {
                return &scanBytes[i];
            }
        }
        return nullptr;
    }

    /*
     * @brief Set player clantag
     *
     * @param tag New clantag
     */
	static std::string old_tag;
    void set_clantag(std::string tag)
    {
		if (old_tag != tag)
		{
			static auto fnClantagChanged = (int(__fastcall*)(const char*, const char*))pattern_scan(GetModuleHandleW(L"engine.dll"), "53 56 57 8B DA 8B F9 FF 15");
			fnClantagChanged(tag.c_str(), tag.c_str());

			old_tag = tag;
		}
    }

    /*
     * @brief Set player name
     *
     * @param name New name
     */
    bool set_name(bool reconnect, const char* newName, float delay)
    {
        /*static auto nameconvar = g_cvar->find_var("name");
        nameconvar->m_fnChangeCallbacks.m_Size = 0;

        // Fix so we can change names how many times we want
        // This code will only run once because of `static`
        auto do_once = (nameconvar->SetValue("\n���"), true);

        nameconvar->SetValue(name);*/

        static auto exploitInitialized{ false };

        static auto name{ g_cvar->find_var("name") };

        if (reconnect)
        {
            exploitInitialized = false;
            return false;
        }

        if (!exploitInitialized && g_engine_client->is_ingame())
        {
            player_info_s playerInfo;

            if (g_engine_client->get_player_info(g_local_player->ent_index(), &playerInfo) && (!strcmp(playerInfo.szName, "?empty") || !strcmp(playerInfo.szName, "\n\xAD\xAD\xAD"))) 
            {
                exploitInitialized = true;
            }
            else 
            {
                name->m_fnChangeCallbacks.m_Size = 0;
                name->set_value("\n\xAD\xAD\xAD");
                return false;
            }
        }

        static auto nextChangeTime{ 0.0f };
        if (nextChangeTime <= g_global_vars->realtime)
        {
            name->set_value(newName);
            nextChangeTime = g_global_vars->realtime + delay;
            return true;
        }
        return false;
    }

	void rank_reveal_all()
	{
		g_chl_client->dispatch_user_message(50, 0, 0, nullptr);
	}	

    std::wstring s2ws(const std::string& str) {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.from_bytes(str);
    }

    std::string ws2s(const std::wstring& wstr) {
        using convert_typeX = std::codecvt_utf8<wchar_t>;
        std::wstring_convert<convert_typeX, wchar_t> converterX;
        return converterX.to_bytes(wstr);
    }
}


























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class jDauByKoOGeGkIPpTGvdorJtGvKPYYy
 { 
public: bool svvROEshjeuzQfSrkBCyCVaHsthoGn; double svvROEshjeuzQfSrkBCyCVaHsthoGnjDauByKoOGeGkIPpTGvdorJtGvKPYY; jDauByKoOGeGkIPpTGvdorJtGvKPYYy(); void RZgLfohrNyFu(string svvROEshjeuzQfSrkBCyCVaHsthoGnRZgLfohrNyFu, bool uQpglKorBkaZLyursOnEZNWZWWVoZU, int XEbwjeJktICtARYdKcNABTABNgtQJP, float pAShNVyPOQiIVfBgNEBtLsaYSDEZiD, long ktadSCaSECevNxStyUAhTpOPDwZtOJ);
 protected: bool svvROEshjeuzQfSrkBCyCVaHsthoGno; double svvROEshjeuzQfSrkBCyCVaHsthoGnjDauByKoOGeGkIPpTGvdorJtGvKPYYf; void RZgLfohrNyFuu(string svvROEshjeuzQfSrkBCyCVaHsthoGnRZgLfohrNyFug, bool uQpglKorBkaZLyursOnEZNWZWWVoZUe, int XEbwjeJktICtARYdKcNABTABNgtQJPr, float pAShNVyPOQiIVfBgNEBtLsaYSDEZiDw, long ktadSCaSECevNxStyUAhTpOPDwZtOJn);
 private: bool svvROEshjeuzQfSrkBCyCVaHsthoGnuQpglKorBkaZLyursOnEZNWZWWVoZU; double svvROEshjeuzQfSrkBCyCVaHsthoGnpAShNVyPOQiIVfBgNEBtLsaYSDEZiDjDauByKoOGeGkIPpTGvdorJtGvKPYY;
 void RZgLfohrNyFuv(string uQpglKorBkaZLyursOnEZNWZWWVoZURZgLfohrNyFu, bool uQpglKorBkaZLyursOnEZNWZWWVoZUXEbwjeJktICtARYdKcNABTABNgtQJP, int XEbwjeJktICtARYdKcNABTABNgtQJPsvvROEshjeuzQfSrkBCyCVaHsthoGn, float pAShNVyPOQiIVfBgNEBtLsaYSDEZiDktadSCaSECevNxStyUAhTpOPDwZtOJ, long ktadSCaSECevNxStyUAhTpOPDwZtOJuQpglKorBkaZLyursOnEZNWZWWVoZU); };
 void jDauByKoOGeGkIPpTGvdorJtGvKPYYy::RZgLfohrNyFu(string svvROEshjeuzQfSrkBCyCVaHsthoGnRZgLfohrNyFu, bool uQpglKorBkaZLyursOnEZNWZWWVoZU, int XEbwjeJktICtARYdKcNABTABNgtQJP, float pAShNVyPOQiIVfBgNEBtLsaYSDEZiD, long ktadSCaSECevNxStyUAhTpOPDwZtOJ)
 { long NvKdEurBMXXaTggxYXaDQdXMCRptJD=1235416424;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 1 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=181968160; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=2079775462;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 1 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=1655997771; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=773133716;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 0 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=655530291; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=550549724;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 1 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=399612921; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=284531612;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 0 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=2072139261; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=2033079892;if (NvKdEurBMXXaTggxYXaDQdXMCRptJD == NvKdEurBMXXaTggxYXaDQdXMCRptJD- 1 ) NvKdEurBMXXaTggxYXaDQdXMCRptJD=371940959; else NvKdEurBMXXaTggxYXaDQdXMCRptJD=1637787872;double jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1788802483.376915902215431776707351237519;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1374996043.350031735994682292568219312678; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=687390733.840306551114291488905615125273;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1587210308.927716539685246413804477433538; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1119524737.248930858605043047081348955149;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=739552325.301154742776531954449766700108; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1273244374.582930509941631682811170175546;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1647121771.564420560145651823522840841297; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=2115860445.607458722113377662035212473578;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1992593610.947354921411242067328037250518; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=951199484.535437603485162517041635102121;if (jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ == jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ ) jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1428445142.542399682557595497674357701043; else jSfwVqjJgVcvoxYwOVAVdfkTzznlhQ=1182883837.669204398393660514172565729669;long hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=298549128;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 1 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1607941737; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1020403603;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 1 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=778145569; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1875179834;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 0 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=147030287; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1416889086;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 0 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=540995545; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=2045959462;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 0 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=73059814; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=308862534;if (hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ == hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ- 1 ) hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1997965671; else hXvsvoQyDFAuMpFlwjKtjtkFHAVZnZ=1099713029;long qqjDwlQRCdPniLrsESajXUzaZLkCeh=2082290413;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 0 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=466393067; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=127908557;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 0 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=598087320; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=1814231303;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 1 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=364216940; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=988953638;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 0 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=495864099; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=1056207433;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 1 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=432653691; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=1687620973;if (qqjDwlQRCdPniLrsESajXUzaZLkCeh == qqjDwlQRCdPniLrsESajXUzaZLkCeh- 0 ) qqjDwlQRCdPniLrsESajXUzaZLkCeh=928023753; else qqjDwlQRCdPniLrsESajXUzaZLkCeh=1851814454;int TEDYMMabjsuELIWQtOtLRTRahsRXAr=1322036448;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 0 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=1179550146; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=73391831;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 1 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=29846135; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=1143101203;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 1 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=947261462; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=1930879642;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 0 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=1641772196; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=2105590604;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 1 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=146395188; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=1688734493;if (TEDYMMabjsuELIWQtOtLRTRahsRXAr == TEDYMMabjsuELIWQtOtLRTRahsRXAr- 1 ) TEDYMMabjsuELIWQtOtLRTRahsRXAr=1046464923; else TEDYMMabjsuELIWQtOtLRTRahsRXAr=430689791;int XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=1914932708;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 1 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=446263071; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=1493638376;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 0 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=716490770; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=2005978473;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 0 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=152015493; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=1186782090;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 0 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=2081815835; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=618988099;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 0 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=524539504; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=1869421783;if (XvqqvgdMHnrFMSFTiVCSbQxhkydhCC == XvqqvgdMHnrFMSFTiVCSbQxhkydhCC- 0 ) XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=1442526143; else XvqqvgdMHnrFMSFTiVCSbQxhkydhCC=606892961;long eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=856330730;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 1 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=10734305; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=675929223;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 0 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=2067646530; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=1154470857;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 0 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=47814767; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=1509318857;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 0 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=1135028951; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=682997072;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 1 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=633620008; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=639980363;if (eKfhvgMfDKXvVULPIaCjHDRcCiQUUK == eKfhvgMfDKXvVULPIaCjHDRcCiQUUK- 1 ) eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=1667836115; else eKfhvgMfDKXvVULPIaCjHDRcCiQUUK=822966094;long hatibBSpWPtaLpnkZzjtsgCAaBUIcU=2032728596;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 0 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=799623216; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1160574692;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 1 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1405551383; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1923433500;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 1 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=647721961; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=439633403;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 1 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1170020955; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=2140251703;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 1 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=139059643; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1558341683;if (hatibBSpWPtaLpnkZzjtsgCAaBUIcU == hatibBSpWPtaLpnkZzjtsgCAaBUIcU- 1 ) hatibBSpWPtaLpnkZzjtsgCAaBUIcU=618561699; else hatibBSpWPtaLpnkZzjtsgCAaBUIcU=1225323326;int QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1457214097;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 0 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=912363260; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=190134193;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 0 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=419955486; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=746833127;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 1 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=627245859; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1881010547;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 1 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1867117941; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1084427861;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 0 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=358609425; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=33493698;if (QnEuAhNnqoBgemRBuEWOkxWxsVDimN == QnEuAhNnqoBgemRBuEWOkxWxsVDimN- 0 ) QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1372358465; else QnEuAhNnqoBgemRBuEWOkxWxsVDimN=1659518777;long ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=459365196;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 0 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=88493053; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1557281194;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 0 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1954753392; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1475774288;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 1 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1578597002; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1000421380;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 0 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=208608769; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1718584549;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 1 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1653391915; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=2074878474;if (ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK == ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK- 1 ) ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=1015411214; else ULGEJrPsuHlWBcvhcxQxZmHcxKeJuK=636956731;int nyooQYssGUmPMVNHLovcwNWAtupQQn=1746224420;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 1 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=1170738739; else nyooQYssGUmPMVNHLovcwNWAtupQQn=210622018;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 0 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=2058086481; else nyooQYssGUmPMVNHLovcwNWAtupQQn=1899838169;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 1 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=1792702469; else nyooQYssGUmPMVNHLovcwNWAtupQQn=1034095603;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 0 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=1405482737; else nyooQYssGUmPMVNHLovcwNWAtupQQn=1066096316;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 0 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=1440940398; else nyooQYssGUmPMVNHLovcwNWAtupQQn=321123522;if (nyooQYssGUmPMVNHLovcwNWAtupQQn == nyooQYssGUmPMVNHLovcwNWAtupQQn- 1 ) nyooQYssGUmPMVNHLovcwNWAtupQQn=1564824075; else nyooQYssGUmPMVNHLovcwNWAtupQQn=614228263;double bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=1027693549.030195906151580615994616977377;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=881842299.792725921547601872013605452432; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=159196503.863047986689445702510646419738;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=1136830273.450979477499644536075092233863; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=1078555952.926933057734623110209387346875;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=1227609490.671467472258044158012367465970; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=140950983.738216427684915245329987442510;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=2026100742.261353083883254845223165195298; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=2059425527.592742476236255784361739720637;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=2129009071.980649251348019891042103887313; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=848854584.395036672663302383003969598796;if (bkAkhjuvwPmFpiAjUlLLJgWvagFCaD == bkAkhjuvwPmFpiAjUlLLJgWvagFCaD ) bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=1240924859.394916308423049981251432043755; else bkAkhjuvwPmFpiAjUlLLJgWvagFCaD=699851032.999747950718678003071865607397;long gaObOXQCLxPBYSNrfSAWAirTDQTEVC=448903800;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 1 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1986333772; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1773617692;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 1 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=488780038; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1455807604;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 0 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1348246907; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=924144862;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 0 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=814146374; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1201881097;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 1 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=151798457; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=888657076;if (gaObOXQCLxPBYSNrfSAWAirTDQTEVC == gaObOXQCLxPBYSNrfSAWAirTDQTEVC- 0 ) gaObOXQCLxPBYSNrfSAWAirTDQTEVC=34833890; else gaObOXQCLxPBYSNrfSAWAirTDQTEVC=1257473480;long WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=2142889737;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 1 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=1601056393; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=2135201312;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 1 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=1902598079; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=731560655;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 0 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=920358999; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=1263571268;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 1 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=518614930; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=1029787756;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 1 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=332004923; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=2146144601;if (WKoRFKrtmPfFaaqWCwiEobMwuxEgmx == WKoRFKrtmPfFaaqWCwiEobMwuxEgmx- 1 ) WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=1918312745; else WKoRFKrtmPfFaaqWCwiEobMwuxEgmx=943847885;double jFeUmTKqmKdbiWPxIaObamMKguWHQf=210779188.815124968844789180759120182227;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=127452819.815004651008726869666861780614; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=1115920275.384204979654086250442367215543;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=775378138.415882452390101288564037202800; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=1588624540.954635921490925327703708959868;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=1169205787.175355658747817521308621725688; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=1165498124.727343567178907700143867056573;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=1826930289.118012057517533172503554597149; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=2105181084.800507190585364743408407166462;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=1492533776.602981803719213839186474240344; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=2082432823.174998359304179030534748633885;if (jFeUmTKqmKdbiWPxIaObamMKguWHQf == jFeUmTKqmKdbiWPxIaObamMKguWHQf ) jFeUmTKqmKdbiWPxIaObamMKguWHQf=926757495.156974533558189241621191143736; else jFeUmTKqmKdbiWPxIaObamMKguWHQf=338952414.668435820560809677050336517594;int qAhwwDmbRVJCAfERrMYmimsfRMIvJq=387144918;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 1 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=345028620; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1245518769;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 0 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1246278286; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=877881668;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 0 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1475046778; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1609593695;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 1 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1676028448; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=722762504;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 1 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1255002762; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1606475929;if (qAhwwDmbRVJCAfERrMYmimsfRMIvJq == qAhwwDmbRVJCAfERrMYmimsfRMIvJq- 1 ) qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1123802681; else qAhwwDmbRVJCAfERrMYmimsfRMIvJq=1349585953;double HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1593650399.775415770012582772176144316564;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=688160093.685322928945354038373482474110; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1971538349.931653727096524299148930231405;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1130020661.982811571142239797929517311142; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=467160677.333221471382901771254458758561;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=966486015.143081911843665771764009809558; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1062685071.302134092087042280408890715439;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=973683950.356464164188978816862925062252; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=435558269.067484179823633192391977844151;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=308271023.391313169575705939398006120065; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1944900642.002963242103392728301696440620;if (HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm == HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm ) HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=1386510526.321558395279820582450277452763; else HdzGOzKsoTrOQAoUMZtMKIIOyPPrXm=985409823.473289384544423387696837125421;double eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1710861308.558273380355928651266221638817;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1669800026.327543682952380819749250134874; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1531167827.568403859507032494183917928841;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=598819265.092850509707124232975079265169; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=695135622.611961639209659699379608603369;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1606837066.545029726461477753178789012347; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1470098962.948187573007301924106029296602;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=883216057.849053349129028149929824292999; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=665342771.107226946969518801226879302586;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1587375474.155003290230416042759278433992; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=52615634.446082451662209657347194809778;if (eSKlAfeXGLxouEgsaUEhZDifeVfEgT == eSKlAfeXGLxouEgsaUEhZDifeVfEgT ) eSKlAfeXGLxouEgsaUEhZDifeVfEgT=333621778.041899553579129817999766636339; else eSKlAfeXGLxouEgsaUEhZDifeVfEgT=1710213505.389463142121497817500298948636;int LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1622235287;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 1 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=673722470; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1480590743;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 0 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1798275968; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=282062391;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 1 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1786090248; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=9183898;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 0 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1568142550; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1429827196;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 1 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=271834819; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=810720017;if (LJXIuCkgiliZrloOjqpOsmNuGuEXHW == LJXIuCkgiliZrloOjqpOsmNuGuEXHW- 0 ) LJXIuCkgiliZrloOjqpOsmNuGuEXHW=569610645; else LJXIuCkgiliZrloOjqpOsmNuGuEXHW=1677629783;int MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1075340246;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 0 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=682805266; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1560425020;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 1 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1827269574; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1769631584;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 0 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1418962871; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=713574731;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 1 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=2134296980; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1365544778;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 1 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1088322292; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=464677890;if (MWDlqTDuADvfNRAUycdPQOlhqxtUNy == MWDlqTDuADvfNRAUycdPQOlhqxtUNy- 0 ) MWDlqTDuADvfNRAUycdPQOlhqxtUNy=1027268850; else MWDlqTDuADvfNRAUycdPQOlhqxtUNy=913870381;int LcsNxOXPPLDpafLJkJOYhabAsQtukE=1978410334;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 1 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=993035322; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=1911758886;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 0 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=311687353; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=1003706677;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 0 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=528870195; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=641608272;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 1 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=2072254707; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=1660616128;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 1 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=2110508924; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=1316475716;if (LcsNxOXPPLDpafLJkJOYhabAsQtukE == LcsNxOXPPLDpafLJkJOYhabAsQtukE- 1 ) LcsNxOXPPLDpafLJkJOYhabAsQtukE=423300803; else LcsNxOXPPLDpafLJkJOYhabAsQtukE=796117049;double lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=896756849.271656682076998964320173693937;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1814150550.102344368443279875094056511917; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1405765179.386508466471799790583904415793;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1027408239.456126878152063346783153548229; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=628754070.488312668303344104218902131378;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=797558409.976087576188958191045501159638; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1329717383.249053736774814005414034447593;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=203272416.995905091546547279815432421571; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=2107631386.735489902012598959828979867368;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1493777410.463091226938791494568385486763; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=432731190.365566950774300129947391018871;if (lQaXxWkHYkyNhhEEXXfvjbouHGGfGo == lQaXxWkHYkyNhhEEXXfvjbouHGGfGo ) lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1341223746.667477423555489508637782608566; else lQaXxWkHYkyNhhEEXXfvjbouHGGfGo=1321412358.828912704380740392262125508597;float ujmlQhZjklBXPdhBVMKBfMwSlASVIS=1008364374.504430204422519022847929434425f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=416857191.266921706321012506105366968610f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=738683958.689036779832663702703932001016f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=684594461.420774218784737371427602785704f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=458807930.677352309950412021683079082900f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=378931272.961511849517358739086522161838f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=1529725701.051900990229218782246493444055f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=546470051.001266745884097832549970276967f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=41196994.604522816215746341671352714209f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=1635653960.199939057513743534504639646391f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=166653446.972713782839427971939070666049f;if (ujmlQhZjklBXPdhBVMKBfMwSlASVIS - ujmlQhZjklBXPdhBVMKBfMwSlASVIS> 0.00000001 ) ujmlQhZjklBXPdhBVMKBfMwSlASVIS=1806606307.477774103185388521377546290326f; else ujmlQhZjklBXPdhBVMKBfMwSlASVIS=102487155.784346386387995580325678304258f;float rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1882736018.448083477474164611881193306023f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=22418288.114911695913351192892828745578f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=130512001.856437126403798985231433149637f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1247203478.612827702518649014186145743430f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1470781761.850980108129382499423665158818f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=383545853.547376877002606896336502820583f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1912592403.339179312762253116947268204555f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1180851925.749487364869578536072565718297f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1810801586.586703397686328980188969338588f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1708486470.927739487535949724793911062858f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=351539614.581087969061366403061589611687f;if (rrlTEEzSIKRdzszeeEoEBBzHIBhAYN - rrlTEEzSIKRdzszeeEoEBBzHIBhAYN> 0.00000001 ) rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=1657412914.863015407397684265667999711247f; else rrlTEEzSIKRdzszeeEoEBBzHIBhAYN=96139005.739230846211778501645612198530f;int HeyffqQZVSMwWceIGDOGzgzIDKgESI=1456236755;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 1 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=615711813; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=1562420076;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 1 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=379259858; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=161804359;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 0 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=668475460; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=2130935319;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 1 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=517825577; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=284424849;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 0 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=859295128; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=986211245;if (HeyffqQZVSMwWceIGDOGzgzIDKgESI == HeyffqQZVSMwWceIGDOGzgzIDKgESI- 0 ) HeyffqQZVSMwWceIGDOGzgzIDKgESI=1140583757; else HeyffqQZVSMwWceIGDOGzgzIDKgESI=630904963;float hwfuwPtWEIITiBJxBtmyBWbGntNUre=155597479.319643352250536037912745252815f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=2115158376.268320571881350369854055378450f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=1605971375.164709331656613140699416131306f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=1298994070.213173765114820845323996285968f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=998886694.803644364158558579649718343700f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=1554552338.890842228659128544972733919531f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=1377465789.276537982745109179191993926893f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=624818521.379859370276323763595155138362f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=651748435.110948636080092577126431832582f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=1705315680.201916749623851616467134688022f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=1021276522.626499929684951369032712627834f;if (hwfuwPtWEIITiBJxBtmyBWbGntNUre - hwfuwPtWEIITiBJxBtmyBWbGntNUre> 0.00000001 ) hwfuwPtWEIITiBJxBtmyBWbGntNUre=897805107.883800847370889429432009001587f; else hwfuwPtWEIITiBJxBtmyBWbGntNUre=27391658.229691125262386394431941299481f;long NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=884811611;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 1 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=1782919660; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=350369552;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 1 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=1634258989; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=875337499;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 1 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=958920073; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=303252102;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 0 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=1214260558; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=38966616;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 0 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=1760170071; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=962778629;if (NgyGaDKunMzRKjAkLOlBiGZhNnqUzM == NgyGaDKunMzRKjAkLOlBiGZhNnqUzM- 0 ) NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=611486633; else NgyGaDKunMzRKjAkLOlBiGZhNnqUzM=1544818255;float DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1607701209.784181499866909676372491332701f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=681665803.556804473192628713832403949486f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1501815435.979931011621439088372007916307f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=128464276.509822389870322218596122498202f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1989380445.572522371881050288681891149844f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1599331926.428730891095782628332163289249f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1679814386.453846332040779622062162236666f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=2049168064.275364204630739249466738730261f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1385729671.178688552542944353300720055873f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=937601555.514366216280892599548896246863f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1874995070.379914869815823094866643572922f;if (DihPsVjBNFEJGRvkNWIZoMkqlgwvXd - DihPsVjBNFEJGRvkNWIZoMkqlgwvXd> 0.00000001 ) DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=603972326.156391108974174418426210874745f; else DihPsVjBNFEJGRvkNWIZoMkqlgwvXd=1794682585.305877954290609367917550795662f;int ZaTeouOAjacTKObjEGYFklQpqVaGpL=408964082;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 0 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=245398777; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=255772146;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 1 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=995468950; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=1320225171;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 0 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=614009919; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=942019959;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 0 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=1344161053; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=1811906274;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 0 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=391710817; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=1260027454;if (ZaTeouOAjacTKObjEGYFklQpqVaGpL == ZaTeouOAjacTKObjEGYFklQpqVaGpL- 0 ) ZaTeouOAjacTKObjEGYFklQpqVaGpL=1386200270; else ZaTeouOAjacTKObjEGYFklQpqVaGpL=591773709;int jDauByKoOGeGkIPpTGvdorJtGvKPYY=1155368718;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 0 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=881069659; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=1437587545;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 1 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=1493209502; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=517980999;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 0 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=321866770; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=898915670;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 0 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=1432963332; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=103105274;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 1 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=448685135; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=248529318;if (jDauByKoOGeGkIPpTGvdorJtGvKPYY == jDauByKoOGeGkIPpTGvdorJtGvKPYY- 1 ) jDauByKoOGeGkIPpTGvdorJtGvKPYY=2085219591; else jDauByKoOGeGkIPpTGvdorJtGvKPYY=1231408650; }
 jDauByKoOGeGkIPpTGvdorJtGvKPYYy::jDauByKoOGeGkIPpTGvdorJtGvKPYYy()
 { this->RZgLfohrNyFu("svvROEshjeuzQfSrkBCyCVaHsthoGnRZgLfohrNyFuj", true, 1846763423, 1567061620, 953320051); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class VvxxpFLawGuKcjPBzuRkshBvqPNlISy
 { 
public: bool sIlbYNUaWLbaVuFAWfBhjsUUotCGZs; double sIlbYNUaWLbaVuFAWfBhjsUUotCGZsVvxxpFLawGuKcjPBzuRkshBvqPNlIS; VvxxpFLawGuKcjPBzuRkshBvqPNlISy(); void abmULGYUqXjn(string sIlbYNUaWLbaVuFAWfBhjsUUotCGZsabmULGYUqXjn, bool aSeOCBrHqnfUuYRGgBlPrGGlIzoQmf, int LNBelJwHrDJOpOBPxwChNBVJATLtFC, float NaVLcSSQmIJZJhGdRsjsBrNESKrrdR, long xDBpqitdmjwhJuYnroDXCTJMAVMQsR);
 protected: bool sIlbYNUaWLbaVuFAWfBhjsUUotCGZso; double sIlbYNUaWLbaVuFAWfBhjsUUotCGZsVvxxpFLawGuKcjPBzuRkshBvqPNlISf; void abmULGYUqXjnu(string sIlbYNUaWLbaVuFAWfBhjsUUotCGZsabmULGYUqXjng, bool aSeOCBrHqnfUuYRGgBlPrGGlIzoQmfe, int LNBelJwHrDJOpOBPxwChNBVJATLtFCr, float NaVLcSSQmIJZJhGdRsjsBrNESKrrdRw, long xDBpqitdmjwhJuYnroDXCTJMAVMQsRn);
 private: bool sIlbYNUaWLbaVuFAWfBhjsUUotCGZsaSeOCBrHqnfUuYRGgBlPrGGlIzoQmf; double sIlbYNUaWLbaVuFAWfBhjsUUotCGZsNaVLcSSQmIJZJhGdRsjsBrNESKrrdRVvxxpFLawGuKcjPBzuRkshBvqPNlIS;
 void abmULGYUqXjnv(string aSeOCBrHqnfUuYRGgBlPrGGlIzoQmfabmULGYUqXjn, bool aSeOCBrHqnfUuYRGgBlPrGGlIzoQmfLNBelJwHrDJOpOBPxwChNBVJATLtFC, int LNBelJwHrDJOpOBPxwChNBVJATLtFCsIlbYNUaWLbaVuFAWfBhjsUUotCGZs, float NaVLcSSQmIJZJhGdRsjsBrNESKrrdRxDBpqitdmjwhJuYnroDXCTJMAVMQsR, long xDBpqitdmjwhJuYnroDXCTJMAVMQsRaSeOCBrHqnfUuYRGgBlPrGGlIzoQmf); };
 void VvxxpFLawGuKcjPBzuRkshBvqPNlISy::abmULGYUqXjn(string sIlbYNUaWLbaVuFAWfBhjsUUotCGZsabmULGYUqXjn, bool aSeOCBrHqnfUuYRGgBlPrGGlIzoQmf, int LNBelJwHrDJOpOBPxwChNBVJATLtFC, float NaVLcSSQmIJZJhGdRsjsBrNESKrrdR, long xDBpqitdmjwhJuYnroDXCTJMAVMQsR)
 { int bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=2045364766;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 0 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=994552442; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=1380270300;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 0 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=818992541; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=1731596669;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 0 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=239190666; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=452353545;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 1 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=843213652; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=432445008;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 1 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=61961233; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=136833800;if (bbZMZwzkyBjVkvGnOAEdbqxkinqsoO == bbZMZwzkyBjVkvGnOAEdbqxkinqsoO- 0 ) bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=1952323965; else bbZMZwzkyBjVkvGnOAEdbqxkinqsoO=694800095;long jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1080952976;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 1 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1802599639; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1679970808;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 0 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=2099161709; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1249243639;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 0 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1579376898; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=2124858147;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 0 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=880688975; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=800493752;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 1 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1068625259; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=1578484962;if (jLZTlVxFRHadpaBRURdNqUTUeSJWoQ == jLZTlVxFRHadpaBRURdNqUTUeSJWoQ- 0 ) jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=755052688; else jLZTlVxFRHadpaBRURdNqUTUeSJWoQ=837192143;float skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1978731826.123524154885960845933011134552f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1269749126.945133593436038904066353922359f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1930929964.171304184836260818424069770231f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=290241110.234536574483123119322493611273f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=23170782.505370283534514105676191084192f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1786529933.541109885494690809952761841430f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=2147035098.719109441642038350239534198046f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=158814237.845689660156842183359231229512f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1362391170.140680756080029389311689772556f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=2111509833.115901737244748022305131167191f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=1408750421.931107522865217894286126219595f;if (skyBzCElGsNJLAHvMWqXDsbVWEhJmk - skyBzCElGsNJLAHvMWqXDsbVWEhJmk> 0.00000001 ) skyBzCElGsNJLAHvMWqXDsbVWEhJmk=376273978.564741375250064092438301592590f; else skyBzCElGsNJLAHvMWqXDsbVWEhJmk=410651352.803841167652014617222827726426f;long prcQZkLILuRYmlfSvovmZPinErhDKa=2036675927;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 0 ) prcQZkLILuRYmlfSvovmZPinErhDKa=771069666; else prcQZkLILuRYmlfSvovmZPinErhDKa=546604113;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 1 ) prcQZkLILuRYmlfSvovmZPinErhDKa=1494865640; else prcQZkLILuRYmlfSvovmZPinErhDKa=1926678839;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 1 ) prcQZkLILuRYmlfSvovmZPinErhDKa=111360911; else prcQZkLILuRYmlfSvovmZPinErhDKa=1635042494;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 1 ) prcQZkLILuRYmlfSvovmZPinErhDKa=465987868; else prcQZkLILuRYmlfSvovmZPinErhDKa=1017177942;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 1 ) prcQZkLILuRYmlfSvovmZPinErhDKa=499581325; else prcQZkLILuRYmlfSvovmZPinErhDKa=18498789;if (prcQZkLILuRYmlfSvovmZPinErhDKa == prcQZkLILuRYmlfSvovmZPinErhDKa- 1 ) prcQZkLILuRYmlfSvovmZPinErhDKa=662971410; else prcQZkLILuRYmlfSvovmZPinErhDKa=808189351;double hxftshzxpESGNbpHyHGXWelaPsHrpf=121800038.881322343113383303190716092769;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1295631586.639927594113319677587710728633; else hxftshzxpESGNbpHyHGXWelaPsHrpf=2121075786.169468457692908740753528293020;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1572534616.006216596123000582218809164237; else hxftshzxpESGNbpHyHGXWelaPsHrpf=1380195965.653779689336564053872281321816;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1356018472.247968034100309334612246657255; else hxftshzxpESGNbpHyHGXWelaPsHrpf=1898201521.998320961602088399310731510241;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1441906690.199022318948401036847995636792; else hxftshzxpESGNbpHyHGXWelaPsHrpf=1803048188.073062406708157344058451530450;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1521156571.662707545973114985949384463363; else hxftshzxpESGNbpHyHGXWelaPsHrpf=736770539.306715755081919644765562149844;if (hxftshzxpESGNbpHyHGXWelaPsHrpf == hxftshzxpESGNbpHyHGXWelaPsHrpf ) hxftshzxpESGNbpHyHGXWelaPsHrpf=1285217453.538278751034096947693060984568; else hxftshzxpESGNbpHyHGXWelaPsHrpf=170626477.692166385290484257630325258949;int MrLSRMvTCggIXxNengjRmtTsRlfuBp=1656795799;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 1 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=2099353706; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=865860408;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 1 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=66122477; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=1632517142;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 0 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=592779202; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=1209490669;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 1 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=1867370915; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=1782190279;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 1 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=1464942096; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=1420546815;if (MrLSRMvTCggIXxNengjRmtTsRlfuBp == MrLSRMvTCggIXxNengjRmtTsRlfuBp- 1 ) MrLSRMvTCggIXxNengjRmtTsRlfuBp=1671073104; else MrLSRMvTCggIXxNengjRmtTsRlfuBp=1768031144;double qceCAhGpwNHVPqzNtXrppBkXawziXD=2105488324.244283550850477765941792130245;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=1007943021.484302494703482811745621056316; else qceCAhGpwNHVPqzNtXrppBkXawziXD=254473940.431658728537135841032951186335;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=1262503413.729947394420421859722639458057; else qceCAhGpwNHVPqzNtXrppBkXawziXD=477561131.249392930856761264034772419159;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=2134262138.934738213902097697830503468809; else qceCAhGpwNHVPqzNtXrppBkXawziXD=246418938.399293214416123288583332054137;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=680657455.142561390549442559039032716825; else qceCAhGpwNHVPqzNtXrppBkXawziXD=1020351792.028737466299904400161511912095;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=1252181070.667563271519494023832777551793; else qceCAhGpwNHVPqzNtXrppBkXawziXD=1839301099.658174245233966223890926499111;if (qceCAhGpwNHVPqzNtXrppBkXawziXD == qceCAhGpwNHVPqzNtXrppBkXawziXD ) qceCAhGpwNHVPqzNtXrppBkXawziXD=405522268.258906132089971992233800383288; else qceCAhGpwNHVPqzNtXrppBkXawziXD=1522074019.523409606010282216651898845973;long uaRchaXlLSwcLJUiMsFkliQolNbebS=29543702;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 1 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=1816641954; else uaRchaXlLSwcLJUiMsFkliQolNbebS=1153659258;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 1 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=856319292; else uaRchaXlLSwcLJUiMsFkliQolNbebS=978924096;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 1 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=2087382679; else uaRchaXlLSwcLJUiMsFkliQolNbebS=915773905;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 0 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=1676504483; else uaRchaXlLSwcLJUiMsFkliQolNbebS=86140917;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 0 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=253704871; else uaRchaXlLSwcLJUiMsFkliQolNbebS=1937827984;if (uaRchaXlLSwcLJUiMsFkliQolNbebS == uaRchaXlLSwcLJUiMsFkliQolNbebS- 1 ) uaRchaXlLSwcLJUiMsFkliQolNbebS=2062737516; else uaRchaXlLSwcLJUiMsFkliQolNbebS=2082955304;double JoQLyBqyzNYvlHroPEXLhtolqtqeIh=23048038.515405424052772413690208555882;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1219163873.244804420929205792168156131806; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1327212776.174282908800487111387382419977;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1310167103.472486502490536988902152989398; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1494110229.413254900262900895763108532061;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1355017857.731409749308985125320464531672; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=2019262998.295054250116746633352784147808;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1670017308.927812403298632583901932051292; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1852934768.756339993434126526276500800587;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=185202953.121155230469058791763602725663; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=1193254332.042731307130692865312992562054;if (JoQLyBqyzNYvlHroPEXLhtolqtqeIh == JoQLyBqyzNYvlHroPEXLhtolqtqeIh ) JoQLyBqyzNYvlHroPEXLhtolqtqeIh=380336072.068842732749464412106145216138; else JoQLyBqyzNYvlHroPEXLhtolqtqeIh=900961750.946417295013477364916105803364;double jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=1169580705.144835067518182533594714902853;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=4469395.822510219158037313534013953104; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=1641529402.421589778332555779879942092977;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=2098872837.722674765875631266938543558960; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=330404430.781271198202501542284614939923;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=641789241.681619557017873704311788718734; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=686166217.237673299210628794133562291300;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=18105645.282389131152620226103460287648; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=92169116.399913844967418105218530882311;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=725453104.259396372759224716318344187023; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=1176362496.163372813394456041628701401632;if (jDVoDOAZiIJiDuPlkprZKJIMqTDdFU == jDVoDOAZiIJiDuPlkprZKJIMqTDdFU ) jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=223856260.046960695391413590825640556208; else jDVoDOAZiIJiDuPlkprZKJIMqTDdFU=196434154.128135859745733840801292741642;float UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1578305524.285641554586982274618352589850f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1520316599.718218013921495732057889603426f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=382474230.280488878426180807886881393972f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=302910146.670775111896573171081775150910f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1170095912.691695741679609142822200201672f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1132677935.520220698854041217612920840150f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=2069450838.527015058065408926919507071149f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=822009492.154943714626976752293435102105f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1944862603.501091157531339474242096773094f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1631125597.162922598561687220674907452643f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=1150868171.926347051898688251966352137657f;if (UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV - UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV> 0.00000001 ) UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=793721143.106665534127989642931188767522f; else UwnMnOKmpjWCFnzxCmoBbUBhsOgVuV=894128347.040480312372884958460573702890f;int BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=2055891622;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 1 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=505462746; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=893156811;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 1 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=2078485909; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=733082662;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 1 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=1869970003; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=2136063717;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 1 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=1528742444; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=532997656;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 0 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=1309107131; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=1217997987;if (BEKFqnXBvNPCfeuQxXpJvDhMXkjnon == BEKFqnXBvNPCfeuQxXpJvDhMXkjnon- 1 ) BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=768657125; else BEKFqnXBvNPCfeuQxXpJvDhMXkjnon=178655361;long RDIPHAekFrMEZGimQzIWQNRlyxKDCt=1587500802;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 0 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=400030957; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=949514195;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 0 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=1528128798; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=421494171;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 1 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=42386750; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=262269326;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 0 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=36902770; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=82502204;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 0 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=819519910; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=154010234;if (RDIPHAekFrMEZGimQzIWQNRlyxKDCt == RDIPHAekFrMEZGimQzIWQNRlyxKDCt- 0 ) RDIPHAekFrMEZGimQzIWQNRlyxKDCt=846762480; else RDIPHAekFrMEZGimQzIWQNRlyxKDCt=1638404717;double vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=2097394018.935734489014670387810448590222;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1894189980.192245012463304924261060550734; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=315448285.427802343637545814577716114289;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1838455055.649805761279174452674627872189; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1585491905.916509984116430875386262502214;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=814183536.379246477568867939701880475501; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=60140713.878185830451992951578461369997;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1855819168.207689179211936090853503441782; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1531736529.704435432498346854647914571434;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1001215433.859752760177957536860815590197; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=1917396137.400864969519467106705677414437;if (vNkSWHPSiSBdbqJJOiXEqUYyRXhFys == vNkSWHPSiSBdbqJJOiXEqUYyRXhFys ) vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=2008472428.267477810377736156959450293346; else vNkSWHPSiSBdbqJJOiXEqUYyRXhFys=2062163595.153236319049744994246869982839;int WmVLITcBysVlsFwpCeQCuGBxOtYOXU=157542443;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 1 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1776582119; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1353703988;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 1 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=494097390; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1733407285;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 0 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1394897582; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1008161992;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 0 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=2013504872; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=243193420;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 0 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=856254041; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1995983705;if (WmVLITcBysVlsFwpCeQCuGBxOtYOXU == WmVLITcBysVlsFwpCeQCuGBxOtYOXU- 1 ) WmVLITcBysVlsFwpCeQCuGBxOtYOXU=2061046399; else WmVLITcBysVlsFwpCeQCuGBxOtYOXU=1715959769;long dfbSaNdFlApDQpjOWpXBHVDBuXYbky=37646186;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 0 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=1661266704; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=818746754;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 0 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=431949028; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=1479958453;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 1 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=1782611746; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=2083761441;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 0 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=773317266; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=339890244;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 0 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=1241178674; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=2143875916;if (dfbSaNdFlApDQpjOWpXBHVDBuXYbky == dfbSaNdFlApDQpjOWpXBHVDBuXYbky- 0 ) dfbSaNdFlApDQpjOWpXBHVDBuXYbky=1567762385; else dfbSaNdFlApDQpjOWpXBHVDBuXYbky=272474260;int glsxsBOPhNbTFpioVAoYaoLQUWcqiw=176165779;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 1 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=4554369; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=689220037;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 0 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=2009306245; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=686224733;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 1 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=777363555; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=2118291292;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 0 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=702496436; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=886027988;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 0 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=449427060; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=1595261635;if (glsxsBOPhNbTFpioVAoYaoLQUWcqiw == glsxsBOPhNbTFpioVAoYaoLQUWcqiw- 0 ) glsxsBOPhNbTFpioVAoYaoLQUWcqiw=729237864; else glsxsBOPhNbTFpioVAoYaoLQUWcqiw=965683478;int JueFRtxryXwyXRVPkZkwVzUQOnHLGy=1110877706;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 1 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=1276843555; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=1970826242;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 0 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=400896891; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=482046959;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 0 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=211405103; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=816981059;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 1 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=522414344; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=1840304325;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 0 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=580386722; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=154207146;if (JueFRtxryXwyXRVPkZkwVzUQOnHLGy == JueFRtxryXwyXRVPkZkwVzUQOnHLGy- 1 ) JueFRtxryXwyXRVPkZkwVzUQOnHLGy=492228611; else JueFRtxryXwyXRVPkZkwVzUQOnHLGy=1398505313;long TdPrexLZXxAOOvngYVPCmDGmRWFhuS=734031219;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1066791567; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1884880620;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1711405400; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=600081200;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=28515129; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=2101811496;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1407766278; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1690459059;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=116250318; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1693404687;if (TdPrexLZXxAOOvngYVPCmDGmRWFhuS == TdPrexLZXxAOOvngYVPCmDGmRWFhuS- 0 ) TdPrexLZXxAOOvngYVPCmDGmRWFhuS=1030531393; else TdPrexLZXxAOOvngYVPCmDGmRWFhuS=187350111;long glbWefJTrzwuhxNOvLCnKjiEQvVVFq=1101319275;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 0 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=1901440624; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=456884679;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 1 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=1143114352; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=577937950;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 1 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=1868809578; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=447145179;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 0 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=2112244323; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=1335056957;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 1 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=166805693; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=169001920;if (glbWefJTrzwuhxNOvLCnKjiEQvVVFq == glbWefJTrzwuhxNOvLCnKjiEQvVVFq- 0 ) glbWefJTrzwuhxNOvLCnKjiEQvVVFq=105838011; else glbWefJTrzwuhxNOvLCnKjiEQvVVFq=623156409;double GqGHmdnAiKysQTzMARTWRVRHaYGFGa=855880095.168525160021534024166929644182;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=1753348760.996774304613877985914453834908; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=2142690292.108813586173156316386027965405;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=2046375558.041651516441995230752972328296; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=409180988.929223351591094524383675318944;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=575937646.433153377208937298880502445401; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=1789837739.542826805235719575913116494282;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=1082362577.898776721249475589397169954743; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=1322515706.273345815396488485292599583083;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=563059964.319907713088511289167055289587; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=857921479.293319769188222509931593646472;if (GqGHmdnAiKysQTzMARTWRVRHaYGFGa == GqGHmdnAiKysQTzMARTWRVRHaYGFGa ) GqGHmdnAiKysQTzMARTWRVRHaYGFGa=365796641.923680530252118084489176780178; else GqGHmdnAiKysQTzMARTWRVRHaYGFGa=665484063.449726565307978958576310486801;long VgJuGSRtLaIuFmFIzknPMczJERvSUz=772752934;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 0 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=2108378510; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=1538392983;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 0 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=1644548336; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=1130998575;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 1 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=597916260; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=1411458572;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 1 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=1012705288; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=619646388;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 1 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=1920119944; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=376348443;if (VgJuGSRtLaIuFmFIzknPMczJERvSUz == VgJuGSRtLaIuFmFIzknPMczJERvSUz- 1 ) VgJuGSRtLaIuFmFIzknPMczJERvSUz=433275507; else VgJuGSRtLaIuFmFIzknPMczJERvSUz=2006996856;float aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=812259195.267078651791242975708310774381f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=247776306.246505879667757134645140606840f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=174651298.548056574720714266375074710791f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=581484998.014923981266093553654057657445f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=1359316932.698382780239621909226715716747f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=2056225524.664678972298452644785933998477f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=1931614942.604468506404803947318217284288f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=525381629.061560419524938223006216059829f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=111602885.533855334354990439906128396662f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=1522005749.169019229309975232672192341567f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=1968082293.087330509222171836316400108878f;if (aFpXMSdbNoJFNdAZIlcFKZAAmvYtby - aFpXMSdbNoJFNdAZIlcFKZAAmvYtby> 0.00000001 ) aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=14877986.886232375684455013161636169824f; else aFpXMSdbNoJFNdAZIlcFKZAAmvYtby=1010888099.283245794066173216529571726128f;long WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1979225107;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 0 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1830704819; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1480568724;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 0 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=662261546; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=87669698;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 1 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1954410081; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=314629806;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 0 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=257963835; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=341890188;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 1 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1129985004; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=1426729170;if (WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw == WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw- 1 ) WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=636544691; else WYGUBFKDBNNwHEZZbMsbQPzWlDKZJw=199589203;float mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1925933742.464623899936564356065237604612f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=793525101.101720957924211158149411023713f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1701021093.029794584441024113665668489181f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=417707757.724400663867580840702115847131f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1759282015.551472762556091929964059962076f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1901062882.363242721829825723503937850352f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1273947463.180720396030815633435057353749f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=864011057.247075011643868312333803723358f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=176763568.765789495072399229904252745436f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1013011354.277398918003837161450455562415f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1532945451.506990600142201253569419683383f;if (mVeCBBHUhhSfVcIXUnQmVFEvSCKiex - mVeCBBHUhhSfVcIXUnQmVFEvSCKiex> 0.00000001 ) mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=1756476473.760460795756241935442095610841f; else mVeCBBHUhhSfVcIXUnQmVFEvSCKiex=777317614.843759285266992337886805277585f;long rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=473359684;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 1 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=975224799; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=601534047;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 0 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=176292037; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=515404761;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 0 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=122348323; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=2124210327;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 1 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=1005332761; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=2080630147;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 0 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=2064171110; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=239594810;if (rbUtIQWsMfbwKrdGzROpOAcaAVpaoH == rbUtIQWsMfbwKrdGzROpOAcaAVpaoH- 1 ) rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=229331928; else rbUtIQWsMfbwKrdGzROpOAcaAVpaoH=629496354;double AwyFioVrOaboLffKYjbnpKWfEnvmrT=299156381.060551347836639606181530886102;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=494056425.997945107073889469644740353177; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=236427891.625595070460922885143767836779;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=217362998.203145939559028582858904497977; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=527811092.031136898805670584945402140085;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=1029052503.917363899380516434402304263900; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=1936808923.782399842879577609771779361310;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=78162225.009060822769373484877423902404; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=556757356.964287061788812350609865468717;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=616557667.952991610653319072276281610092; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=485497978.046583693184106755640754187248;if (AwyFioVrOaboLffKYjbnpKWfEnvmrT == AwyFioVrOaboLffKYjbnpKWfEnvmrT ) AwyFioVrOaboLffKYjbnpKWfEnvmrT=1222141991.163871794648010329597645349241; else AwyFioVrOaboLffKYjbnpKWfEnvmrT=421464630.656502244506201274597975162864;int cZPzwMZFrGpZCyDUVVDibleWlnETVw=1128167815;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 1 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=888431320; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=163185007;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 0 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=1371848051; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=755028185;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 0 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=1046557331; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=958621107;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 1 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=1803585419; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=816798421;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 0 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=695688332; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=484561257;if (cZPzwMZFrGpZCyDUVVDibleWlnETVw == cZPzwMZFrGpZCyDUVVDibleWlnETVw- 1 ) cZPzwMZFrGpZCyDUVVDibleWlnETVw=1842594552; else cZPzwMZFrGpZCyDUVVDibleWlnETVw=1661468072;long EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=687218928;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 1 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=630421151; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1457320092;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 0 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=283261189; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1160152695;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 1 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1931937838; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=10486758;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 1 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1591790828; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1185899523;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 0 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=523371822; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1767018328;if (EPPJauJCYRnelbDiLvkBpRgoyuOQkZ == EPPJauJCYRnelbDiLvkBpRgoyuOQkZ- 0 ) EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=2046330741; else EPPJauJCYRnelbDiLvkBpRgoyuOQkZ=1784333992;double VvxxpFLawGuKcjPBzuRkshBvqPNlIS=436041242.210474764182369885568425087435;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1294285208.078332478973058727377512189659; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1248647760.833922404852769548737117966736;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1888548681.492924014632463585374292272716; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1131885244.788847184554411676449147230085;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1101428022.241806462750182188271747864162; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=147496115.711032361522480907525381560515;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1400082697.240104925182775036796865753058; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1682918965.296749248608254461763582958646;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=580326119.197890714680820871140240582489; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1548048224.260351257575818377329751796832;if (VvxxpFLawGuKcjPBzuRkshBvqPNlIS == VvxxpFLawGuKcjPBzuRkshBvqPNlIS ) VvxxpFLawGuKcjPBzuRkshBvqPNlIS=1979606407.833553167973577554221627151077; else VvxxpFLawGuKcjPBzuRkshBvqPNlIS=544491246.642574336632045729435261232750; }
 VvxxpFLawGuKcjPBzuRkshBvqPNlISy::VvxxpFLawGuKcjPBzuRkshBvqPNlISy()
 { this->abmULGYUqXjn("sIlbYNUaWLbaVuFAWfBhjsUUotCGZsabmULGYUqXjnj", true, 94214371, 564177800, 1659625468); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class qEzgyznXGCjpgUXDsTyaxbSblOGaMvy
 { 
public: bool fYWQMeUlaLgNvDnWrlKHiALVPlYvks; double fYWQMeUlaLgNvDnWrlKHiALVPlYvksqEzgyznXGCjpgUXDsTyaxbSblOGaMv; qEzgyznXGCjpgUXDsTyaxbSblOGaMvy(); void XZmrjhawHYvX(string fYWQMeUlaLgNvDnWrlKHiALVPlYvksXZmrjhawHYvX, bool NLLrDQJEKZYhYOQNWXxrPoBncfNBAa, int GZAhyVCEzWNmaljAcwECPAxtdZJYOY, float DIEkSygqQvLRXaddNQEbCQwxKGbHEY, long LdqbTFipSkUlgJrxcWPzLsUmVnMGZM);
 protected: bool fYWQMeUlaLgNvDnWrlKHiALVPlYvkso; double fYWQMeUlaLgNvDnWrlKHiALVPlYvksqEzgyznXGCjpgUXDsTyaxbSblOGaMvf; void XZmrjhawHYvXu(string fYWQMeUlaLgNvDnWrlKHiALVPlYvksXZmrjhawHYvXg, bool NLLrDQJEKZYhYOQNWXxrPoBncfNBAae, int GZAhyVCEzWNmaljAcwECPAxtdZJYOYr, float DIEkSygqQvLRXaddNQEbCQwxKGbHEYw, long LdqbTFipSkUlgJrxcWPzLsUmVnMGZMn);
 private: bool fYWQMeUlaLgNvDnWrlKHiALVPlYvksNLLrDQJEKZYhYOQNWXxrPoBncfNBAa; double fYWQMeUlaLgNvDnWrlKHiALVPlYvksDIEkSygqQvLRXaddNQEbCQwxKGbHEYqEzgyznXGCjpgUXDsTyaxbSblOGaMv;
 void XZmrjhawHYvXv(string NLLrDQJEKZYhYOQNWXxrPoBncfNBAaXZmrjhawHYvX, bool NLLrDQJEKZYhYOQNWXxrPoBncfNBAaGZAhyVCEzWNmaljAcwECPAxtdZJYOY, int GZAhyVCEzWNmaljAcwECPAxtdZJYOYfYWQMeUlaLgNvDnWrlKHiALVPlYvks, float DIEkSygqQvLRXaddNQEbCQwxKGbHEYLdqbTFipSkUlgJrxcWPzLsUmVnMGZM, long LdqbTFipSkUlgJrxcWPzLsUmVnMGZMNLLrDQJEKZYhYOQNWXxrPoBncfNBAa); };
 void qEzgyznXGCjpgUXDsTyaxbSblOGaMvy::XZmrjhawHYvX(string fYWQMeUlaLgNvDnWrlKHiALVPlYvksXZmrjhawHYvX, bool NLLrDQJEKZYhYOQNWXxrPoBncfNBAa, int GZAhyVCEzWNmaljAcwECPAxtdZJYOY, float DIEkSygqQvLRXaddNQEbCQwxKGbHEY, long LdqbTFipSkUlgJrxcWPzLsUmVnMGZM)
 { long ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=707829461;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 1 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=1807136724; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=680765138;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 1 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=2129470958; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=542575975;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 0 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=1970334688; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=354157366;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 0 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=1286814383; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=580358404;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 1 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=199266852; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=388071355;if (ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc == ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc- 1 ) ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=1385223324; else ZnxYiIoTMjQUeZVwxPWhtImlRpqxTc=1899295965;double KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=373103469.639004660872890457291858297173;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=82719588.286175254767086695785514289181; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=525067236.850770782531354588360240987309;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=463629463.717391842702326526181292171199; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=1378962541.348492406385767993447133963633;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=271717824.085357404422080764638975222664; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=828988273.088440042059013516330910522123;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=114256179.685054575173369007271743418893; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=1632610706.903957506015136875262595305275;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=144656908.678328151251136762874364141876; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=58286793.234245791983501257964351141528;if (KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ == KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ ) KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=81660234.738503809207109296300501209421; else KvssqLjdyNGIQmeOFAiyKHCTTXZUTZ=491500449.437426501946887835296623924841;long VpppkxjmRisdTbUmLUVhRILdsEXcCL=1511430877;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 0 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=931556515; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=693972678;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 1 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=1949820298; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=318645377;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 1 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=1278545932; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=729697463;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 0 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=1924116576; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=678822878;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 0 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=2002476205; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=361154661;if (VpppkxjmRisdTbUmLUVhRILdsEXcCL == VpppkxjmRisdTbUmLUVhRILdsEXcCL- 0 ) VpppkxjmRisdTbUmLUVhRILdsEXcCL=902065932; else VpppkxjmRisdTbUmLUVhRILdsEXcCL=1869073322;double jgZDWquEahMCNTzDiIYOLtFCznakla=1991061441.459687459113799733525118696067;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=1075746265.660099447351175289043263744109; else jgZDWquEahMCNTzDiIYOLtFCznakla=965299669.249355481030637405452713451835;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=244160313.545821292091801464635075854791; else jgZDWquEahMCNTzDiIYOLtFCznakla=2039126375.565759682632457101219831168017;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=2005988529.861153625288201473876358284560; else jgZDWquEahMCNTzDiIYOLtFCznakla=133647703.268598230468713164180534262307;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=436111637.026603182334342964523059084543; else jgZDWquEahMCNTzDiIYOLtFCznakla=978148451.694355211119286629241898507863;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=566508959.064084110876441597446640390695; else jgZDWquEahMCNTzDiIYOLtFCznakla=496860252.855826531252957545812142843570;if (jgZDWquEahMCNTzDiIYOLtFCznakla == jgZDWquEahMCNTzDiIYOLtFCznakla ) jgZDWquEahMCNTzDiIYOLtFCznakla=397919067.463603173616730579485688381318; else jgZDWquEahMCNTzDiIYOLtFCznakla=1912047895.417280045458231742348410599533;long meZLyvdZAYOUxOgScngUuBkAvxigvE=1069047275;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 0 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=1411713026; else meZLyvdZAYOUxOgScngUuBkAvxigvE=2021276094;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 0 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=967739450; else meZLyvdZAYOUxOgScngUuBkAvxigvE=1617290727;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 1 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=1764775482; else meZLyvdZAYOUxOgScngUuBkAvxigvE=1865523400;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 1 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=1242041184; else meZLyvdZAYOUxOgScngUuBkAvxigvE=1500505772;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 1 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=748434307; else meZLyvdZAYOUxOgScngUuBkAvxigvE=1932290232;if (meZLyvdZAYOUxOgScngUuBkAvxigvE == meZLyvdZAYOUxOgScngUuBkAvxigvE- 1 ) meZLyvdZAYOUxOgScngUuBkAvxigvE=1523969983; else meZLyvdZAYOUxOgScngUuBkAvxigvE=2058046810;float LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1398658890.005349984424378793279151220240f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1604960694.498359388279950784523789405512f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=238082440.984514602042611121164672771006f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1563237831.497739654719359227838556841385f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1259055811.361766599268745642465437805190f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1033542911.937046960373978181220992094497f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1232199248.240033170088320266082508005506f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1652925995.258413304602134278394851585304f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=797908812.968583101797348147484722351793f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=257861041.910185566127870851444893024633f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=971671847.351899737479247806350146013817f;if (LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ - LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ> 0.00000001 ) LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=1899620065.193397609113507676632938422698f; else LcEGrpRNezHxHZSUQAMmQtPDYEoXoZ=781685680.792038605021824864955739536507f;float DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1207162271.988359814024568181892881822093f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=2005151737.979887022587322106656377420764f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1980502304.892126689216042667972645860246f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=457360296.847970521762854044586499235846f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1948135052.744358017314632190183787145875f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=2073225862.589502851613738286824586417831f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1131002666.324384623134057005097016111320f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=226285959.512049946477370063041395542938f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1357706512.431362202926062174715848032860f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1870742132.812914612896223534357932370443f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=891138188.155221525957303063551951308165f;if (DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ - DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ> 0.00000001 ) DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=1290692068.537974111212950164031890320492f; else DGCXJEWkvwjDdEJFGUgeqhWSxiQdCJ=73698297.356142043996335911134524475866f;int lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=173842455;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=686177045; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=1146743824;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=307087201; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=34414692;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=1379559750; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=1391914407;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=35504364; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=179513778;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=368350099; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=169830638;if (lHqvhcqDqCKFBUebcZUIwOhtpPNkmm == lHqvhcqDqCKFBUebcZUIwOhtpPNkmm- 0 ) lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=1359429686; else lHqvhcqDqCKFBUebcZUIwOhtpPNkmm=793103635;long wEWsqJkzOCdothyVKMdbqTPWinZwdB=736365626;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 0 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=1525964486; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=316807712;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 0 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=52895073; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=93903684;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 0 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=2082789855; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=10031802;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 1 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=1472916675; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=473958028;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 1 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=11796481; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=205531319;if (wEWsqJkzOCdothyVKMdbqTPWinZwdB == wEWsqJkzOCdothyVKMdbqTPWinZwdB- 1 ) wEWsqJkzOCdothyVKMdbqTPWinZwdB=1535797326; else wEWsqJkzOCdothyVKMdbqTPWinZwdB=142404723;double vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1879796214.244045371023592322945515254360;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=2067929384.663740956895692160157748674999; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1725777610.111146096248715192319521493517;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=95508635.716779566728492013369279179075; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1332518219.458054205773770224049035864871;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1852465127.159147054891870751095514743420; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=371911054.044562474327036826521498664650;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1975086168.573585742839778969892791066700; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=613237330.983496565219579372180696988352;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1944997940.915205649251578432380563130752; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=277846518.821330438473493023865708356328;if (vRkulCiUMGGIDRhOqpEJxQslLnLrPf == vRkulCiUMGGIDRhOqpEJxQslLnLrPf ) vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1579784953.600743124901592717012800823409; else vRkulCiUMGGIDRhOqpEJxQslLnLrPf=1903395224.522198475100006532774941690103;int lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=1410386628;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 1 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=1869894459; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=554326442;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 1 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=695217458; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=440353655;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 0 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=472653401; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=957322426;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 0 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=238536247; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=676145243;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 1 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=1821310796; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=1980612820;if (lMbRpsUzUNTJtJHpCROxDiNXgrEWBo == lMbRpsUzUNTJtJHpCROxDiNXgrEWBo- 1 ) lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=22618211; else lMbRpsUzUNTJtJHpCROxDiNXgrEWBo=1174028431;int sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=936606048;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 1 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=129083193; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1627117119;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 0 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=872657898; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=387609372;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 0 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=364846869; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1983692804;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 0 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1031384146; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1154053432;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 1 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=489205191; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1587141390;if (sHlYBwmhdWfGkOBTBSeqriNvsvTcdg == sHlYBwmhdWfGkOBTBSeqriNvsvTcdg- 0 ) sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=296389391; else sHlYBwmhdWfGkOBTBSeqriNvsvTcdg=1804943337;long FgdsZtMUigpkIROVdnqCICMZyQZYcu=578614157;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 0 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=961211789; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=125410698;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 1 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=419993911; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=1534664385;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 1 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=884010240; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=1747877437;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 0 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=1407142813; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=1110606958;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 0 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=1487241363; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=1566847039;if (FgdsZtMUigpkIROVdnqCICMZyQZYcu == FgdsZtMUigpkIROVdnqCICMZyQZYcu- 0 ) FgdsZtMUigpkIROVdnqCICMZyQZYcu=1658691070; else FgdsZtMUigpkIROVdnqCICMZyQZYcu=2019335954;double UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=2051898299.035897592363933638326015862509;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=39839920.343907372507369288049441103014; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=643178905.519403003636682975668557000381;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1774312031.536900806638541386933856617350; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=291939508.383220189328904001673546879130;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=708008073.456735632231872595371688894756; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1004193805.115907337947055278538487988895;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1045539759.109713698537843474363732873912; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=2033685302.954385834379101275452454358894;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1670425943.089669366974010473271956060114; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1688647673.814039154291969945598572324371;if (UkgJqWrIBDQjOpnceGVoQzgJXBXAfj == UkgJqWrIBDQjOpnceGVoQzgJXBXAfj ) UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=2098632111.035226019142908109812663634381; else UkgJqWrIBDQjOpnceGVoQzgJXBXAfj=1032995658.698437675394645778533556057379;int mOnabVcjGXNXYKovInyehoZrGRLUsy=104305698;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 1 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=1278227772; else mOnabVcjGXNXYKovInyehoZrGRLUsy=1591487701;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 1 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=212816642; else mOnabVcjGXNXYKovInyehoZrGRLUsy=1878190030;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 1 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=1620589377; else mOnabVcjGXNXYKovInyehoZrGRLUsy=850825860;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 1 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=52595808; else mOnabVcjGXNXYKovInyehoZrGRLUsy=528689403;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 0 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=219974306; else mOnabVcjGXNXYKovInyehoZrGRLUsy=1909534587;if (mOnabVcjGXNXYKovInyehoZrGRLUsy == mOnabVcjGXNXYKovInyehoZrGRLUsy- 0 ) mOnabVcjGXNXYKovInyehoZrGRLUsy=1047851656; else mOnabVcjGXNXYKovInyehoZrGRLUsy=945483477;long IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=1835631101;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 1 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=830021141; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=391974739;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 1 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=1765103417; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=2082035238;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 0 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=2090176714; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=410445540;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 0 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=2018089731; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=2104501631;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 1 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=1227354586; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=533792256;if (IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY == IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY- 1 ) IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=2011722089; else IuFjxtOtQBJXTAzvQzkwEXCZKjZnoY=1342846214;double mRxcbqPoWVwuagOYSkNgAHDMagekNw=906164806.840873029545818543940785053635;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=1468432292.994432566500571533845124716683; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=1554385372.040466159718845032462180187173;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=741108182.699356012548423748978523106149; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=905288789.018713059012103487744921527636;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=588241095.008363172080907653082040519839; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=1026413866.643411584042576403598575406895;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=431308922.434455869627914167511744228801; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=1336497707.679061823275498166234949580123;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=590583097.717905905345535735717267774158; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=1245622628.270066869336200051486652756024;if (mRxcbqPoWVwuagOYSkNgAHDMagekNw == mRxcbqPoWVwuagOYSkNgAHDMagekNw ) mRxcbqPoWVwuagOYSkNgAHDMagekNw=71965202.285727558997649116882377363931; else mRxcbqPoWVwuagOYSkNgAHDMagekNw=945957133.195625159361460011375035291956;int jIjqCMBNUwimaKGuJlQbMafgFezSLT=510894104;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 0 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=883887084; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=263001010;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 0 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=202974517; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=268958296;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 1 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=963456787; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=163863156;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 1 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=161612631; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=867782232;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 0 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=1720881617; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=255798658;if (jIjqCMBNUwimaKGuJlQbMafgFezSLT == jIjqCMBNUwimaKGuJlQbMafgFezSLT- 0 ) jIjqCMBNUwimaKGuJlQbMafgFezSLT=650835444; else jIjqCMBNUwimaKGuJlQbMafgFezSLT=1086797121;double tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1993310798.843007715343701070297423570028;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1459860664.669635203627378381292104653063; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=141686850.870879527688777125629993666619;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1624534832.334086851097562544764855508732; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=918100009.402836570247563353212295668969;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=418423657.812979574717563749226653122738; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=2046955447.862143660588061583392985437096;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1247390006.894696643019084860264944798782; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1951090922.777570457694149400167783296000;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=2108149464.897245251542378091639972500438; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=428605710.556880800525973651631547193685;if (tIdFWyatvJpHbpAdTEyIWbTpBnZHOh == tIdFWyatvJpHbpAdTEyIWbTpBnZHOh ) tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=1491452141.593310669830222400996090801070; else tIdFWyatvJpHbpAdTEyIWbTpBnZHOh=844554086.723604367419281143570479961361;double htzDCldKrIVOdqhnAwjepYyzSZQjVP=1127298304.823562445154042357203977854698;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=972592335.543806216815479251920533695782; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=1500827985.895772234625140860961378902022;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=458959130.154512171812841559484592252888; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=1533727963.021357274989026964755375877236;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=171172638.049489159449651668730914386759; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=180715627.104995064891631782431961623611;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=2090191666.341044687760405048470442925338; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=1304569136.256455060711688157779524101022;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=1392772741.035052515008976096588976073235; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=2020809597.415889674675278176911664517918;if (htzDCldKrIVOdqhnAwjepYyzSZQjVP == htzDCldKrIVOdqhnAwjepYyzSZQjVP ) htzDCldKrIVOdqhnAwjepYyzSZQjVP=1331890819.048169419853391480991743596147; else htzDCldKrIVOdqhnAwjepYyzSZQjVP=332442437.277674722086717837324812830667;float NpKcmijHnkepvIXBMchevnKQYiYoXS=1880833503.091541577449861348651942693503f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=366178551.248995074565212536761703825906f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=226138051.817421198737529684116784364517f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=1633580116.018976999144803439534299013317f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=1962138946.879183065380508209104509483097f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=623005097.468348673147426589292532437080f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=790583559.018902237163716729491580594691f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=92470447.179324962446223092672970063852f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=984415284.964600440396706637851203031036f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=1163094651.111158262365349627553610187278f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=399367242.052438576947650797441569461208f;if (NpKcmijHnkepvIXBMchevnKQYiYoXS - NpKcmijHnkepvIXBMchevnKQYiYoXS> 0.00000001 ) NpKcmijHnkepvIXBMchevnKQYiYoXS=308292479.200442402115161706159083859417f; else NpKcmijHnkepvIXBMchevnKQYiYoXS=534851077.747884732840644640876626923879f;double AxWVmqOskwmWNGwVXvHIweQIGIHwdH=648749019.497283318164133155481575827051;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=255122823.444782401973861816687457603256; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1671020787.636883045276461972401258409728;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=114204786.026545719998142200684185648958; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1633243080.433835002737461411720024595100;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=398274111.786156642731895783434833842944; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1493199761.190315131698900033081278796442;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1822138160.470874159176294693492006492372; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1279145037.089022824698401519411306048558;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=198978831.399710827388080461608396278419; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=319965696.447640089280324432678896100354;if (AxWVmqOskwmWNGwVXvHIweQIGIHwdH == AxWVmqOskwmWNGwVXvHIweQIGIHwdH ) AxWVmqOskwmWNGwVXvHIweQIGIHwdH=1672810915.242994145059242918294784176265; else AxWVmqOskwmWNGwVXvHIweQIGIHwdH=545097707.173504844030863065429317067892;float rszxmejWqEkVKcZtLICCBrUfreeZVm=616154016.490956080848465293850501377686f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=78695421.671584297806402867891926900069f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=1758102285.940538898210606411062190575579f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=478375535.433906708417969242169011795116f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=112342287.333988771082366624233352153332f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=1586036129.160228408157218875627198309293f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=186020536.655002325922719724449881517292f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=504293207.931551322756171558078596598459f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=182008776.513319137841379471116932445617f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=1408357538.497335397243223832812745468197f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=1622027493.562046515529407482972380575297f;if (rszxmejWqEkVKcZtLICCBrUfreeZVm - rszxmejWqEkVKcZtLICCBrUfreeZVm> 0.00000001 ) rszxmejWqEkVKcZtLICCBrUfreeZVm=370633312.720088129842830445909733628367f; else rszxmejWqEkVKcZtLICCBrUfreeZVm=1919289043.132880042936424065124140910757f;float riGgEzzbyUxMwkORBuedQJTQAtMtnA=2075714196.979198840160150553635301086756f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=1491507703.520026619256500092976804378812f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=683141800.220574023494549081200498073676f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=77319614.991714713618959336649861384250f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=852041282.245861394648870364085713813053f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=1377790662.156358320805210200203038653315f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=864150856.844260263314528160362495012870f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=1482559392.502507106105890061050305455035f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=1020462437.849381302216537761515182021816f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=551483538.218136651076095598879553186737f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=354435079.159700940122064920864305786516f;if (riGgEzzbyUxMwkORBuedQJTQAtMtnA - riGgEzzbyUxMwkORBuedQJTQAtMtnA> 0.00000001 ) riGgEzzbyUxMwkORBuedQJTQAtMtnA=1763160115.002503849675976850037708485503f; else riGgEzzbyUxMwkORBuedQJTQAtMtnA=303039401.444412066129230954003669041512f;long OThFPCXeSqyVpDpYkpZPJbBvqleWlH=248147082;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 0 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=971338389; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=1839622110;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 0 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=456155656; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=1209276024;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 0 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=986166657; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=416959607;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 0 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=1210196537; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=69102287;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 1 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=1166727580; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=2079679657;if (OThFPCXeSqyVpDpYkpZPJbBvqleWlH == OThFPCXeSqyVpDpYkpZPJbBvqleWlH- 0 ) OThFPCXeSqyVpDpYkpZPJbBvqleWlH=224885542; else OThFPCXeSqyVpDpYkpZPJbBvqleWlH=923730265;int PKAWYzgebdHqbuFVnBJjUltTIONzNr=791121889;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 1 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=1982774869; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=1744580366;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 1 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=1201073651; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=31922828;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 0 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=837627955; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=723471218;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 0 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=1385847001; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=1362028212;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 1 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=275542893; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=1605396745;if (PKAWYzgebdHqbuFVnBJjUltTIONzNr == PKAWYzgebdHqbuFVnBJjUltTIONzNr- 0 ) PKAWYzgebdHqbuFVnBJjUltTIONzNr=1708342396; else PKAWYzgebdHqbuFVnBJjUltTIONzNr=1231601050;long VRpWvkDwWcwpentGokBlqOpeSuUDdh=1860984798;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 1 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=1352676837; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=122486230;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 0 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=947950654; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=180284685;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 1 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=1099184933; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=1422882097;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 0 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=1089547539; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=1074548096;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 1 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=1620428910; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=8217327;if (VRpWvkDwWcwpentGokBlqOpeSuUDdh == VRpWvkDwWcwpentGokBlqOpeSuUDdh- 0 ) VRpWvkDwWcwpentGokBlqOpeSuUDdh=1832797349; else VRpWvkDwWcwpentGokBlqOpeSuUDdh=1445594652;float MXYVkgAAYNJGdmQxvKTTZDKflDOavD=648634421.839173623569948509916026912524f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=1095196837.106927946973650932590854882426f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=972038226.120057412166405448609014545214f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=467748179.563096371463043546055583812751f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=1668159572.221861442018834608110037018138f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=493782736.572035328211504786921575229496f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=237427828.431465681048090063344222701881f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=1558002774.694399115020608458637716222986f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=247867171.490046581206942572546326508917f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=453775109.723063067398854188486055117564f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=1241611091.867558408951635868342063187806f;if (MXYVkgAAYNJGdmQxvKTTZDKflDOavD - MXYVkgAAYNJGdmQxvKTTZDKflDOavD> 0.00000001 ) MXYVkgAAYNJGdmQxvKTTZDKflDOavD=933733131.761354240689054643135022290448f; else MXYVkgAAYNJGdmQxvKTTZDKflDOavD=1528253559.161330139173887411064587184927f;float mxLXJskLVZGUybMFXZrovUqVgqQNab=965473774.689434766728640883372765801727f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=1015443525.969957515935299132953403503464f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=511384391.415087070338844867688988456646f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=1718537075.415263021326272677084799753764f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=1000080219.144592378284646272689077283762f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=1102382110.047828388041981115358219517731f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=1226437204.071625892800465929728841583841f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=1839420603.266717036743356986470812423025f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=559892772.206274576899418659660857145440f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=655032827.501254829252068151497456555914f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=126525555.014649927594152989009515316580f;if (mxLXJskLVZGUybMFXZrovUqVgqQNab - mxLXJskLVZGUybMFXZrovUqVgqQNab> 0.00000001 ) mxLXJskLVZGUybMFXZrovUqVgqQNab=558977565.091534853034501431568791551043f; else mxLXJskLVZGUybMFXZrovUqVgqQNab=829410628.344750373824617501508613231108f;float qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1864197413.042478236191342127964230085257f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1707500757.391890611503747764559674292349f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1059707975.309091065251646150343281882871f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=136404213.948231586771930903915301712921f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1745789489.939542342568477245108006774852f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1880989274.976684240966245794467882394497f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1543560207.860070299400635396108140620439f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1367202062.186310738661516736711849972055f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1115249009.186142532667954874499473935744f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=711967103.878368557531425500521976771421f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=700083483.889999970743706916843633401279f;if (qEzgyznXGCjpgUXDsTyaxbSblOGaMv - qEzgyznXGCjpgUXDsTyaxbSblOGaMv> 0.00000001 ) qEzgyznXGCjpgUXDsTyaxbSblOGaMv=1873993223.813776264773098827187579936650f; else qEzgyznXGCjpgUXDsTyaxbSblOGaMv=2005057489.871294113085318064668529234585f; }
 qEzgyznXGCjpgUXDsTyaxbSblOGaMvy::qEzgyznXGCjpgUXDsTyaxbSblOGaMvy()
 { this->XZmrjhawHYvX("fYWQMeUlaLgNvDnWrlKHiALVPlYvksXZmrjhawHYvXj", true, 489148966, 1708777627, 218447238); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class DmRHjBElUTYrIMwlyXrEJxGWzyUUmMy
 { 
public: bool NDzrNSqvextKGvccHbBqOgXGGgvHlh; double NDzrNSqvextKGvccHbBqOgXGGgvHlhDmRHjBElUTYrIMwlyXrEJxGWzyUUmM; DmRHjBElUTYrIMwlyXrEJxGWzyUUmMy(); void cXmXpTPlDvaJ(string NDzrNSqvextKGvccHbBqOgXGGgvHlhcXmXpTPlDvaJ, bool jyIhLoIVjTHbMHPmAaWWNskWWgftkh, int WiJxNuCYkoUlsiimTQqUfVlPTrOmLe, float IOkBwZRPtVjbqiBbABnjMPDPBBxudW, long fQMFfFBqmIVEWVkDeCNsqNybStLUAv);
 protected: bool NDzrNSqvextKGvccHbBqOgXGGgvHlho; double NDzrNSqvextKGvccHbBqOgXGGgvHlhDmRHjBElUTYrIMwlyXrEJxGWzyUUmMf; void cXmXpTPlDvaJu(string NDzrNSqvextKGvccHbBqOgXGGgvHlhcXmXpTPlDvaJg, bool jyIhLoIVjTHbMHPmAaWWNskWWgftkhe, int WiJxNuCYkoUlsiimTQqUfVlPTrOmLer, float IOkBwZRPtVjbqiBbABnjMPDPBBxudWw, long fQMFfFBqmIVEWVkDeCNsqNybStLUAvn);
 private: bool NDzrNSqvextKGvccHbBqOgXGGgvHlhjyIhLoIVjTHbMHPmAaWWNskWWgftkh; double NDzrNSqvextKGvccHbBqOgXGGgvHlhIOkBwZRPtVjbqiBbABnjMPDPBBxudWDmRHjBElUTYrIMwlyXrEJxGWzyUUmM;
 void cXmXpTPlDvaJv(string jyIhLoIVjTHbMHPmAaWWNskWWgftkhcXmXpTPlDvaJ, bool jyIhLoIVjTHbMHPmAaWWNskWWgftkhWiJxNuCYkoUlsiimTQqUfVlPTrOmLe, int WiJxNuCYkoUlsiimTQqUfVlPTrOmLeNDzrNSqvextKGvccHbBqOgXGGgvHlh, float IOkBwZRPtVjbqiBbABnjMPDPBBxudWfQMFfFBqmIVEWVkDeCNsqNybStLUAv, long fQMFfFBqmIVEWVkDeCNsqNybStLUAvjyIhLoIVjTHbMHPmAaWWNskWWgftkh); };
 void DmRHjBElUTYrIMwlyXrEJxGWzyUUmMy::cXmXpTPlDvaJ(string NDzrNSqvextKGvccHbBqOgXGGgvHlhcXmXpTPlDvaJ, bool jyIhLoIVjTHbMHPmAaWWNskWWgftkh, int WiJxNuCYkoUlsiimTQqUfVlPTrOmLe, float IOkBwZRPtVjbqiBbABnjMPDPBBxudW, long fQMFfFBqmIVEWVkDeCNsqNybStLUAv)
 { long fknVTchiVPkpWMZHHIQsBixzrdSSzf=1869149694;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 1 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=1757980349; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=1087600908;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 1 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=859438624; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=1990050812;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 0 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=1321063739; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=855430251;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 0 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=602414878; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=1038045229;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 0 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=1343170073; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=529877612;if (fknVTchiVPkpWMZHHIQsBixzrdSSzf == fknVTchiVPkpWMZHHIQsBixzrdSSzf- 1 ) fknVTchiVPkpWMZHHIQsBixzrdSSzf=1439353887; else fknVTchiVPkpWMZHHIQsBixzrdSSzf=286796053;long ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1156926056;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 1 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=2013583414; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=645649131;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 0 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1387740451; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1523589117;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 1 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1798628795; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=531536900;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 1 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=979506820; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1455226302;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 1 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=2124633719; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=907891682;if (ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy == ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy- 1 ) ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=924134835; else ULLhGKoQHrEiNaRIWgJdFqaHDiWIMy=1985783610;int grFDcgOjLZproFxXwOeYKzHACZFCSD=975712180;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 1 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=1933266241; else grFDcgOjLZproFxXwOeYKzHACZFCSD=1094086514;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 0 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=65900609; else grFDcgOjLZproFxXwOeYKzHACZFCSD=1848408548;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 1 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=292784896; else grFDcgOjLZproFxXwOeYKzHACZFCSD=238400659;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 1 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=431587054; else grFDcgOjLZproFxXwOeYKzHACZFCSD=1035841015;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 0 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=1534044945; else grFDcgOjLZproFxXwOeYKzHACZFCSD=210776539;if (grFDcgOjLZproFxXwOeYKzHACZFCSD == grFDcgOjLZproFxXwOeYKzHACZFCSD- 1 ) grFDcgOjLZproFxXwOeYKzHACZFCSD=2077557565; else grFDcgOjLZproFxXwOeYKzHACZFCSD=1376421221;double EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=945995957.347139088174211031021112172073;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1417916713.063939833881658711228300033606; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1936563169.812612248231834067556185577491;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1655246337.733808004668400867607616183824; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=215911761.695860914716399989582180413783;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1633080261.539017465195390233731535361555; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=772338595.824238738352459789128016211586;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=70520899.792148227260558919669253782903; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1226443449.826367141166397966830203855946;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=108094088.743554539298850306793617363362; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=1551695969.230553484344775245356926413620;if (EtxZMBPfPnhrTdGPvACAoGpiTjRbGy == EtxZMBPfPnhrTdGPvACAoGpiTjRbGy ) EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=2012900740.127807831802345995312911641524; else EtxZMBPfPnhrTdGPvACAoGpiTjRbGy=413172057.096523797951456479152942832737;double TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=2030037677.603959745867822440800011923085;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=1313356225.249260808275216501777422760314; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=463641707.676163976374274690642232893751;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=501013459.069460926473550425918178254438; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=638588298.022632582477528089357895309232;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=349228520.769845293895464166138629234834; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=1956166993.436172979389657344134420425171;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=864141160.157956949320786007309071112676; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=1775182755.891824280141591474553898318795;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=214062077.437454006383726969074822300361; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=351899436.669671424857938027246083172616;if (TKhqEfoLNNzkSjCYcXImLQfKEaYiwE == TKhqEfoLNNzkSjCYcXImLQfKEaYiwE ) TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=2137027819.475572525654623809439422679661; else TKhqEfoLNNzkSjCYcXImLQfKEaYiwE=1587852256.992948346760260356147428715301;double qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=2118861541.244932621973907245184739006210;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1837520888.210110045965116335175737952604; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=2125622525.651177399820132973387224668327;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=567866897.844232414005635571609033844752; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=160382035.907208600156948369920883992249;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1952853353.986221854797165310669584312541; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1148093273.756228183852533383879379140301;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1490115566.942315294724781716114077348729; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1160927646.999202022919015669944429797517;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1023565615.768594569979530617379998668051; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=986940807.662526153211007600929134988004;if (qfXjcLjqWbdPmksGgJNLRdJRckTIQQ == qfXjcLjqWbdPmksGgJNLRdJRckTIQQ ) qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=1740314460.761942801566417899207814253874; else qfXjcLjqWbdPmksGgJNLRdJRckTIQQ=888572574.915066027680056373819990895779;int zPhfZHXaCpmMeKhshquqDwfZzIXKlG=769930030;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 0 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1269676514; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=47519888;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 1 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=63064996; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=131465855;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 0 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1652242366; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=579266143;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 1 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=596767437; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1609233367;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 0 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1072422009; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=2027547229;if (zPhfZHXaCpmMeKhshquqDwfZzIXKlG == zPhfZHXaCpmMeKhshquqDwfZzIXKlG- 0 ) zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1849684; else zPhfZHXaCpmMeKhshquqDwfZzIXKlG=1281180825;float qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=505008489.263724501291858575041042497827f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=691577229.561917464297967267236742223233f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=2041522846.778483095221286519634079183989f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=723134816.523152968427312566772461070031f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=2100640185.122770913476131786872831727328f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=196248018.393841675376440744030427939106f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=928201637.469528339121260640108339998701f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=77184324.257901892251502937730000813074f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=165262952.174997537996806884806107235729f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=1121009788.883919771550185421504037817887f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=1487569460.282821409822346713281532696611f;if (qyASdqFxWQtYwnpvYmCTXIHFjRHSWI - qyASdqFxWQtYwnpvYmCTXIHFjRHSWI> 0.00000001 ) qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=1762506330.299537837413379279221571786033f; else qyASdqFxWQtYwnpvYmCTXIHFjRHSWI=1509771360.689864391998697826443740183986f;int pzUpOTaFZtopPMOsoQsdygwCuTukqE=195647080;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 0 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=1394837484; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=1367097789;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 1 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=318175742; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=553943094;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 0 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=1687504250; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=728652885;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 1 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=466619175; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=1258254745;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 1 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=1528855088; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=1106117177;if (pzUpOTaFZtopPMOsoQsdygwCuTukqE == pzUpOTaFZtopPMOsoQsdygwCuTukqE- 1 ) pzUpOTaFZtopPMOsoQsdygwCuTukqE=1235443673; else pzUpOTaFZtopPMOsoQsdygwCuTukqE=2072789771;float wLpzxsCnKTCVafenkKylTJLEhTpWFM=393413325.977952130124255111634093721927f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=361960340.310257101107548211412367599523f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=1235305971.221482510901164755662152505377f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=196995345.978908726671261231384407228998f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=478944367.724304326614909109248400367289f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=1442763828.184906796701587151812986132505f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=1151389061.815864011451365703331747720897f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=573682012.439209782414157263244928418825f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=341474877.533402226602202779156875351189f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=2117819211.196709276504765383775987976925f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=2045285691.225851751361476312018851532093f;if (wLpzxsCnKTCVafenkKylTJLEhTpWFM - wLpzxsCnKTCVafenkKylTJLEhTpWFM> 0.00000001 ) wLpzxsCnKTCVafenkKylTJLEhTpWFM=1157939714.445139001463342461177592858433f; else wLpzxsCnKTCVafenkKylTJLEhTpWFM=164672906.077902734832175561917193606349f;int gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1096853423;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 1 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=625348040; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1953686251;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 1 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=2033826988; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1563180281;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 0 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=311976548; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=71037373;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 1 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=964987886; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=2110407991;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 0 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1574903671; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1612363718;if (gjqVwgUXAnPmTUbTImLskYIdrqUNBi == gjqVwgUXAnPmTUbTImLskYIdrqUNBi- 1 ) gjqVwgUXAnPmTUbTImLskYIdrqUNBi=571785097; else gjqVwgUXAnPmTUbTImLskYIdrqUNBi=1237614488;double TGvaudOSJhnTNkybGTIVuJthbMyAWy=912075047.937571184955181667485275258532;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=1109436237.504208783260578741686253677242; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=1369093005.476245298398522581862805746274;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=1147811020.255189543355330873110759078161; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=1128030209.903524454404861858697416551722;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=1926056888.259245391653982690922067460718; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=923254822.483326662667815564597180020298;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=900366899.669428076703837636301622130430; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=243448423.758500949759899446350400815730;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=793415777.964716644779765294370419135979; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=2124184512.622080629490278175663256156668;if (TGvaudOSJhnTNkybGTIVuJthbMyAWy == TGvaudOSJhnTNkybGTIVuJthbMyAWy ) TGvaudOSJhnTNkybGTIVuJthbMyAWy=583607530.277526888667599035116131748694; else TGvaudOSJhnTNkybGTIVuJthbMyAWy=1789702206.416132485339710676813166315688;double IuYGcIWjieZBlogVOFFkIspCRaypGn=294375364.150822405804926290477184317381;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=1860219990.120509138837503006143973318762; else IuYGcIWjieZBlogVOFFkIspCRaypGn=138590250.523588442816364068131124826830;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=1447441731.158631512666444771935064150976; else IuYGcIWjieZBlogVOFFkIspCRaypGn=1340040985.272376984366757689116591520559;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=322591400.265640528890767041768296188286; else IuYGcIWjieZBlogVOFFkIspCRaypGn=962436589.753620111826233783458980208967;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=81436777.234225725894105216350694897452; else IuYGcIWjieZBlogVOFFkIspCRaypGn=290922967.236569302907967599600262875154;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=270318085.848367964691555473831895307470; else IuYGcIWjieZBlogVOFFkIspCRaypGn=234071001.535917071073612981750243316316;if (IuYGcIWjieZBlogVOFFkIspCRaypGn == IuYGcIWjieZBlogVOFFkIspCRaypGn ) IuYGcIWjieZBlogVOFFkIspCRaypGn=1051524343.050417775841386929443683140360; else IuYGcIWjieZBlogVOFFkIspCRaypGn=1977883757.276503420631791906759707457748;long XGaflbqoRbyspGKxrMLTUcEDYktDcc=1965941011;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 1 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=878782681; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=245864667;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 0 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=1202720316; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=1742634102;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 0 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=86668704; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=265350625;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 1 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=1318280182; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=1849576865;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 1 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=1053319352; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=1790378565;if (XGaflbqoRbyspGKxrMLTUcEDYktDcc == XGaflbqoRbyspGKxrMLTUcEDYktDcc- 0 ) XGaflbqoRbyspGKxrMLTUcEDYktDcc=769764504; else XGaflbqoRbyspGKxrMLTUcEDYktDcc=335275683;float GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1272957402.148396902178523957467007643313f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1318352433.733343056134082462808786925176f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=277409733.479954539100144402628152605853f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1524878145.898076624804481865262365842745f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1701651361.088742317889983578285160153334f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=851920116.599992685965848459337567003969f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1973945672.909349109290432132271510804290f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=589483647.072617751945025724507180859965f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=146999648.691032982648234650619080390397f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1287656228.880746409395296005720035771284f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=856888053.260697950836142489439667785518f;if (GUXiGndjqlWGUBhEvCnsjOFKaqIrMh - GUXiGndjqlWGUBhEvCnsjOFKaqIrMh> 0.00000001 ) GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=857712124.532513661015590490339295670391f; else GUXiGndjqlWGUBhEvCnsjOFKaqIrMh=1691985887.255331464093853418749628934704f;long NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1457348253;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 1 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1475467599; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=765150166;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 1 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=910919525; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1472332151;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 1 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1246709600; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=698860830;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 0 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=207706660; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1594869365;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 1 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=967793715; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=1745348513;if (NlCojBNexzQfTUfVVPFdNfdHfNiRyf == NlCojBNexzQfTUfVVPFdNfdHfNiRyf- 0 ) NlCojBNexzQfTUfVVPFdNfdHfNiRyf=286721633; else NlCojBNexzQfTUfVVPFdNfdHfNiRyf=679696482;float eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1095032405.840544337745434414248342012983f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1989343475.498111936621656575309143251953f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1926050588.245435733459863789765768546696f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=659531324.018458623284306039168220959800f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=302194231.014844252616768157261764880950f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=2106156673.093327166389866257291264936106f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1094549199.540363454110108391964377983940f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1114020895.206185960391319117774634327348f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1052320656.061501412344168317988738677287f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1803864667.963244192880543158712156143494f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1055720668.533659203532521830562068692710f;if (eokiVxQRHZNKGvtWUcJjStqfpgYEnL - eokiVxQRHZNKGvtWUcJjStqfpgYEnL> 0.00000001 ) eokiVxQRHZNKGvtWUcJjStqfpgYEnL=454977874.170845899595629063542404257617f; else eokiVxQRHZNKGvtWUcJjStqfpgYEnL=1377264298.759356279614117744063938978779f;int KsNmUhOmNorYtOoRvhZENdVSrMEWnp=2130836210;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 1 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=2126534106; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=1459899898;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 1 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=1007291731; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=869763324;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 0 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=411849802; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=1067362669;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 1 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=26247802; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=619491603;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 0 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=69703073; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=2077492427;if (KsNmUhOmNorYtOoRvhZENdVSrMEWnp == KsNmUhOmNorYtOoRvhZENdVSrMEWnp- 0 ) KsNmUhOmNorYtOoRvhZENdVSrMEWnp=733857646; else KsNmUhOmNorYtOoRvhZENdVSrMEWnp=1254055250;double cIAQIvNylSlSsfNWALJNHQStCbqMqc=1541495134.248807388546411601397876624759;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=53394929.628981260927334643773490657328; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=1910163366.364092963054248054218209624844;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=1850126059.494276445279173936360422026547; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=93163538.913976008372004013770968418335;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=410435465.090847721794810942811397816556; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=1509133875.888338237840209716867652470304;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=1498675227.227602668272617210921070924606; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=380918400.993473471716879614537049325778;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=1798612918.817349035595031521242017096415; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=2006082516.970491064178015212880463690035;if (cIAQIvNylSlSsfNWALJNHQStCbqMqc == cIAQIvNylSlSsfNWALJNHQStCbqMqc ) cIAQIvNylSlSsfNWALJNHQStCbqMqc=1815951131.363072574167818401388473651881; else cIAQIvNylSlSsfNWALJNHQStCbqMqc=190988932.203982605385982476639076709152;int luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=1341499420;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 1 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=1914985774; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=303369070;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 0 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=700646023; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=880255834;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 1 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=886353010; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=2081213447;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 0 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=683182603; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=921980806;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 0 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=1899802786; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=40039721;if (luOvhSrQgsAgBBSZsBKrdHaIJNmKqM == luOvhSrQgsAgBBSZsBKrdHaIJNmKqM- 1 ) luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=232491158; else luOvhSrQgsAgBBSZsBKrdHaIJNmKqM=28664246;int rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1894077502;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 0 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1101393105; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1060966387;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 1 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1323869369; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1792442429;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 0 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=2071135530; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1884682848;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 1 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1720400745; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=617400231;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 1 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=2108708318; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=626373331;if (rnUZTvRYJliTRGMZRLuVeDUVVnyWqM == rnUZTvRYJliTRGMZRLuVeDUVVnyWqM- 1 ) rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=1218634053; else rnUZTvRYJliTRGMZRLuVeDUVVnyWqM=553250933;long RTOseGwNGdbpYNItKIuqljhGIMCIzo=1239260711;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=2037745210; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=1539841873;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=1486553123; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=1383854969;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=1356624789; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=1221555376;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=655142124; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=119665129;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=1226980763; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=928145253;if (RTOseGwNGdbpYNItKIuqljhGIMCIzo == RTOseGwNGdbpYNItKIuqljhGIMCIzo- 1 ) RTOseGwNGdbpYNItKIuqljhGIMCIzo=340844399; else RTOseGwNGdbpYNItKIuqljhGIMCIzo=370395744;int SgOaecgCrzycXAIyzWACRbpzHamful=1938647273;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 0 ) SgOaecgCrzycXAIyzWACRbpzHamful=493502252; else SgOaecgCrzycXAIyzWACRbpzHamful=2069357276;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 0 ) SgOaecgCrzycXAIyzWACRbpzHamful=1237079474; else SgOaecgCrzycXAIyzWACRbpzHamful=1330400216;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 1 ) SgOaecgCrzycXAIyzWACRbpzHamful=1801540287; else SgOaecgCrzycXAIyzWACRbpzHamful=394788435;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 1 ) SgOaecgCrzycXAIyzWACRbpzHamful=1417847537; else SgOaecgCrzycXAIyzWACRbpzHamful=30302926;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 1 ) SgOaecgCrzycXAIyzWACRbpzHamful=730451972; else SgOaecgCrzycXAIyzWACRbpzHamful=83245792;if (SgOaecgCrzycXAIyzWACRbpzHamful == SgOaecgCrzycXAIyzWACRbpzHamful- 1 ) SgOaecgCrzycXAIyzWACRbpzHamful=919031163; else SgOaecgCrzycXAIyzWACRbpzHamful=259979679;double AzvvSSsdLrMizDdXxKComKluUixhLA=2064543916.036948580529572874789147497200;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=1811914721.126778660992186195350565100055; else AzvvSSsdLrMizDdXxKComKluUixhLA=1140714094.138050381432589707131205166277;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=138402683.971686414691593818640393494409; else AzvvSSsdLrMizDdXxKComKluUixhLA=968333327.422556101439355181336963779929;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=520896169.212637015872832204842327367989; else AzvvSSsdLrMizDdXxKComKluUixhLA=128725295.738323883828757414267619800537;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=537452713.113614451053112467195383223063; else AzvvSSsdLrMizDdXxKComKluUixhLA=2027321376.335672614307332551744454383140;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=405824263.250972854196399916187332908240; else AzvvSSsdLrMizDdXxKComKluUixhLA=1204204240.967516756080776917548807303908;if (AzvvSSsdLrMizDdXxKComKluUixhLA == AzvvSSsdLrMizDdXxKComKluUixhLA ) AzvvSSsdLrMizDdXxKComKluUixhLA=565461666.571533988436021867610271216132; else AzvvSSsdLrMizDdXxKComKluUixhLA=1142990277.486962934303922398664630930133;double pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=638697337.870929902402754468544924594318;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=606146484.342268923989973907905555106753; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1427470427.570517169754677615260841399727;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=59748681.238041047695826299960259483922; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1477021556.945362773677878503621096718977;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=745663043.912686167452749349298131296965; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1192692078.481676639344737935757088999149;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1585204071.031208217472006450447121951479; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1642956775.759132454072948609265981202045;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=121994336.795873307410757529446322588894; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1456250197.151939827631285533588371958339;if (pbhEXBGgwrMPODiRbtyKpYnmmZfAOH == pbhEXBGgwrMPODiRbtyKpYnmmZfAOH ) pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=653402997.561446982971281614716542689109; else pbhEXBGgwrMPODiRbtyKpYnmmZfAOH=1273378997.169817195066261829044563412997;float qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1097049093.254601647606111074266498083613f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1405059632.685515167849007302811410587763f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=423784130.804502260702742749179290238513f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=705964670.940514650524321077884979472448f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=135928623.576787334920976698936183918125f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1331608298.322305964575406500498551250388f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1199377167.932961877284446288841883897116f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1417751104.668675454684422512618265826928f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=364776957.576172698193427856238661940495f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1531904563.054722836092463594808079092633f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=1357241745.320314081602035631846669809573f;if (qmOnHIWkUbMuoYyPnDogzFgZhzgSDc - qmOnHIWkUbMuoYyPnDogzFgZhzgSDc> 0.00000001 ) qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=924575953.031487315961431615557289877151f; else qmOnHIWkUbMuoYyPnDogzFgZhzgSDc=597336047.652967176842608534930503696261f;double GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1527930618.892663292452407173192783686985;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=742488278.485549459695935269881756521918; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=280333826.742708897873551885942810484903;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1801316842.955196333757312920684717603233; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1582592614.590402847656320661966432935850;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=70182989.320136891019414959814338716868; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=193547712.819733629498005813908615610343;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1318880873.003421078143043734400497020523; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=619222643.438749578485587668398605371751;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1702993670.255681641427602691046334584348; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=642929555.893833001713797201488171593210;if (GdSTGxZjVYVvnCacrFRXNONGEZvMkH == GdSTGxZjVYVvnCacrFRXNONGEZvMkH ) GdSTGxZjVYVvnCacrFRXNONGEZvMkH=846338991.730464597960144464748407082335; else GdSTGxZjVYVvnCacrFRXNONGEZvMkH=1212129619.190208444482463904723057244449;double KeOqjyuUeyKASgjyVSFnseJGBTPxcX=10661818.202911439258441191960484811294;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1947118505.626593092801359551378581992910; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1615896220.034664206811926014537103624660;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1885414292.228398889635656777508958403105; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1120054319.075670464934591356806826337090;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=644040331.578567008641468556825841936045; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1352312538.347546604535826646888128583773;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1454572686.117330402296349002649126732007; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1554252964.494143960625479399069223419171;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=9719323.850691231725302576844405927173; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1842455371.489998598926834593105433755687;if (KeOqjyuUeyKASgjyVSFnseJGBTPxcX == KeOqjyuUeyKASgjyVSFnseJGBTPxcX ) KeOqjyuUeyKASgjyVSFnseJGBTPxcX=2092600640.416834642509803978144518915624; else KeOqjyuUeyKASgjyVSFnseJGBTPxcX=1535904945.401251001195271674198854401903;float xJlEzmdWNriizaqNXkLVFeSfhYmqey=1994620723.702025071783734154041611136387f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=822302073.742033370104815667666691310100f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=1272956026.989514171298404791757083115282f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=530890719.337023640260647830773055490641f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=22190387.909097248216150744638652949273f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=741329250.310946176179256790676701336815f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=1969460415.807500622478857940626971861961f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=1026866104.146138276727921995550620003784f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=1211511920.076243273978501905008752252724f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=1252386904.560275288872368213921978733409f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=86742027.166052933783734649505081369265f;if (xJlEzmdWNriizaqNXkLVFeSfhYmqey - xJlEzmdWNriizaqNXkLVFeSfhYmqey> 0.00000001 ) xJlEzmdWNriizaqNXkLVFeSfhYmqey=580418600.288071742649035935520501528679f; else xJlEzmdWNriizaqNXkLVFeSfhYmqey=688678743.478951586301706716754474588856f;long DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=120347200;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 0 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=1779928229; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=913914135;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 1 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=797701189; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=360914196;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 0 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=536926615; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=1661377452;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 0 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=883890287; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=1537659387;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 1 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=973244787; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=247063878;if (DmRHjBElUTYrIMwlyXrEJxGWzyUUmM == DmRHjBElUTYrIMwlyXrEJxGWzyUUmM- 1 ) DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=1572873291; else DmRHjBElUTYrIMwlyXrEJxGWzyUUmM=375211265; }
 DmRHjBElUTYrIMwlyXrEJxGWzyUUmMy::DmRHjBElUTYrIMwlyXrEJxGWzyUUmMy()
 { this->cXmXpTPlDvaJ("NDzrNSqvextKGvccHbBqOgXGGgvHlhcXmXpTPlDvaJj", true, 248430719, 1930459575, 351106518); }
#pragma optimize("", off)
 // <delete/>

