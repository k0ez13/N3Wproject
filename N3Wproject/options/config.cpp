#include "config.h"
#include "options.h"
#include "../helpers/math.h"
#include "../helpers/utils.h"
#include "../menu/menu.h"
#include "../helpers/input.h"
#include "../imgui/imgui_internal.h"
#include "../features/notify/notify.h"

char line_splited = '/';

namespace config
{
	namespace c_utils
	{
		std::string split_line(std::string temp)
		{
			std::string new_t = "";

			for (int g = 0; g < temp.length(); g++)
			{
				if (temp[g] != line_splited)
				{
					new_t += temp[g];
				}
				else
				{
					return new_t;
				}
			}
		}

		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<window_v>& vars, std::string name, std::string wind, float& x, float& y)
		{
			add_space(cur_group, config);

			std::string full = name + " = x-" + std::to_string((int)x) + " y-" + std::to_string((int)y) + ";";

			vars.push_back(window_v{ full, name + " = ", wind, x, y, cur_line, cur_group });
			config.push_back(name + " = x-" + std::to_string((int)x) + " y-" + std::to_string((int)y) + ";" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<string_v>& vars, std::string name, std::string& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = '" + var + "';";

			vars.push_back(string_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = '" + var + "';" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<int_v>& vars, std::string name, int& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + std::to_string(var) + ";";

			vars.push_back(int_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + std::to_string(var) + ";" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<char_v>& vars, std::string name, char& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + std::to_string(var) + ";";

			vars.push_back(char_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + std::to_string(var) + ";" + "\n");

			cur_line++;
		}

		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<float_v>& vars, std::string name, float& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + std::to_string(var) + ";";

			vars.push_back(float_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + std::to_string(var) + ";" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<bind_v>& vars, std::string name, KeyBind_t& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + std::to_string(var.key_id) + "-" + std::to_string(var.bind_type) + "-" + (var.enable ? "true" : "false") + ";";

			vars.push_back(bind_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + std::to_string(var.key_id) + "-" + std::to_string(var.bind_type) + "-" + (var.enable ? "true" : "false") + ";" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<bool_v>& vars, std::string name, bool& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + (var ? "true" : "false") + ";";

			vars.push_back(bool_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + (var ? "true" : "false") + ";" + "\n");

			cur_line++;
		}
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<color_v>& vars, std::string name, Color& var)
		{
			add_space(cur_group, config);

			std::string full = name + " = " + std::to_string(var.r()) + "-" + std::to_string(var.g()) + "-" + std::to_string(var.b()) + "-" + std::to_string(var.a()) + ";";

			vars.push_back(color_v{ full, name + " = ", var, cur_line, cur_group });
			config.push_back(name + " = " + std::to_string(var.r()) + "-" + std::to_string(var.g()) + "-" + std::to_string(var.b()) + "-" + std::to_string(var.a()) + ";" + "\n");

			cur_line++;
		}
		

		

		void add_space(int& cur_group, std::vector<std::string>& config)
		{
			for (int i = 0; i < cur_group; i++)
			{
				std::string text = "| ";

				if (i == cur_group - 1)
					text = "|- ";
				else
					text = "|  ";

				config.push_back(text);
			}
		}

		void add_group(int& cur_line, int& cur_group, std::vector<std::string>& config, std::string name)
		{
			add_space(cur_group, config);

			config.push_back("[ " + name + " ]" + "\n");

			cur_group++;
			cur_line++;
		}

		void end_group(int& cur_group)
		{
			cur_group--;
		}

		void save(std::string path, std::vector<std::string>& config, std::string name)
		{
			if (name.empty())
				return;
			CreateDirectoryA((LPCSTR)std::string("C:\\N3W\\" + path + "\\").c_str(), NULL);

			std::ofstream out;
			out.open("C:\\N3W\\" + path + "\\" + name + ".cfg");
			if (out.is_open())
			{
				for (int i = 0; i < config.size(); i++)
				{
					out << config[i];
				}
			}
			out.close();
		}

		void load(std::string path, std::string name, std::vector<int_v>& ints, std::vector<bool_v>& bools, std::vector<float_v>& floats, std::vector<color_v>& colors, std::vector<char_v>& chars, std::vector<bind_v>& binds, std::vector<string_v>& strings, std::vector<window_v>& windows)
		{
			if (name.empty())
				return;

			CreateDirectoryA((LPCSTR)std::string("C:\\N3W\\" + path + "\\").c_str(), NULL);

			std::string line;

			std::string config = "";

			std::ifstream in("C:\\N3W\\" + path + "\\" + name + ".cfg");

			if (in.is_open())
			{
				while (getline(in, line))
				{
					config += line;
					config += line_splited;
				}
			}
			in.close();


			for (int i = 0; i < bools.size(); i++)
			{
				if (strstr(config.c_str(), bools[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(bools[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, bools[i].text.length());
					new_t.erase(new_t.length() - 1, 1);

					bools[i].v = (new_t == "true" ? true : false);
				}
			}

			for (int i = 0; i < ints.size(); i++)
			{
				if (strstr(config.c_str(), ints[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(ints[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, ints[i].text.length());
					new_t.erase(new_t.length() - 1, 1);

					ints[i].v = std::stoi(new_t);

				}
			}

			for (int i = 0; i < floats.size(); i++)
			{
				if (strstr(config.c_str(), floats[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(floats[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, floats[i].text.length());
					new_t.erase(new_t.length() - 1, 1);

					floats[i].v = std::stof(new_t);
				}
			}

			for (int i = 0; i < strings.size(); i++)
			{
				if (strstr(config.c_str(), strings[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(strings[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, strings[i].text.length() + 1);
					new_t.erase(new_t.length() - 2, 2);

					strings[i].v = new_t;
				}
			}

			for (int i = 0; i < colors.size(); i++)
			{
				if (strstr(config.c_str(), colors[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(colors[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, colors[i].text.length());

					std::string color = "";

					char split = '-';
					char endl = ';';

					float col[4] = { 0, 0, 0, 0 };

					int h = 0;

					for (int g = 0; g < new_t.length(); g++)
					{

						if (new_t[g] != split && new_t[g] != endl)
						{
							color += new_t[g];
						}
						else
						{
							col[h] = std::stof(color);
							color = "";
							h++;
						}
					}



					colors[i].v._CColor[0] = col[0];
					colors[i].v._CColor[1] = col[1];
					colors[i].v._CColor[2] = col[2];
					colors[i].v._CColor[3] = col[3];

					colors[i].v.save_color[0] = col[0] / 255.f;
					colors[i].v.save_color[1] = col[1] / 255.f;
					colors[i].v.save_color[2] = col[2] / 255.f;
					colors[i].v.save_color[3] = col[3] / 255.f;
				}
			}

			for (int i = 0; i < binds.size(); i++)
			{
				if (strstr(config.c_str(), binds[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(binds[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, binds[i].text.length());

					std::string value = "";

					int h = 0;

					char split = '-';
					char endl = ';';

					for (int g = 0; g < new_t.length(); g++)
					{
						if (new_t[g] != split && new_t[g] != endl)
						{
							value += new_t[g];
						}
						else
						{
							switch (h)
							{
							case 0: binds[i].v.key_id = std::stoi(value);
							case 1: binds[i].v.bind_type = std::stoi(value);
							case 2: binds[i].v.enable = (value == "true" ? true : false);
							}

							value = "";
							h++;
						}
					}
				}
			}

			for (int i = 0; i < windows.size(); i++)
			{

				if (strstr(config.c_str(), windows[i].text.c_str()))
				{
					std::string temp = config;
					temp.erase(0, temp.find(windows[i].text));

					std::string new_t = c_utils::split_line(temp);
					new_t.erase(0, windows[i].text.length() + 2);

					ImVec2 local_pos = ImVec2(0, 0);

					char split = ' ';
					char endl = ';';

					std::string value = "";

					for (int g = 0; g < new_t.length(); g++)
					{
						if (new_t[g] != split && new_t[g] != endl)
						{
							value += new_t[g];
						}
						else if (new_t[g] == split)
						{
							local_pos.x = std::stof(value);
							g += 2;

							value = "";
						}
						else if (new_t[g] == endl)
						{
							local_pos.y = std::stof(value);

							value = "";
						}
					}

					ImGui::SetWindowPos(windows[i].window_name.c_str(), local_pos);
				}

			}
		}
	}

	namespace standart
	{
		void clear()
		{
			config.clear();

			ints.clear();
			bools.clear();
			binds.clear();
			floats.clear();
			colors.clear();
			strings.clear();
			windows.clear();

			cur_line = 0;
			cur_group = 0;
		}

		void setup_set()
		{
			clear();

			add_group("legit bot");
			{
				add_group("general");
				{
					add_var("[lb general] bind check", settings::legit_bot::bind_check);
					add_var("[lb general] bind", settings::legit_bot::bind);
					add_var("[lb general] auto-current", settings::legit_bot::auto_current);
					add_var("[lb general] settings weapon", settings::legit_bot::settings_weapon);
					add_var("[lb general] settings category", settings::legit_bot::settings_category);
					add_var("[lb general] settings group", settings::legit_bot::weapon_group);
				}
				end_group();

				add_group("category");
				{
					add_group("weapons");
					{
						for (auto& [id, val] : weapon_names)
						{
							auto& settings_cur = settings::legit_bot::legitbot_items[id];

							add_group(val);
							{
								add_var("[lb weap]-[" + val + "] enable", settings_cur.enabled);

								add_group("fov's");
								{
									add_var("[lb weap]-[fov]-[" + val + "] fov", settings_cur.fov);
									add_var("[lb weap]-[fov]-[" + val + "] silent", settings_cur.silent);
									add_var("[lb weap]-[fov]-[" + val + "] silent fov", settings_cur.silent_fov);
								}
								end_group();

								add_var("[lb weap]-[" + val + "] deathmatch", settings_cur.deathmatch);
								add_var("[lb weap]-[" + val + "] autopistol", settings_cur.autopistol);

								add_group("check's");
								{
									add_var("[lb weap]-[check]-[" + val + "] smoke check", settings_cur.smoke_check);
									add_var("[lb weap]-[check]-[" + val + "] flash check", settings_cur.flash_check);
									add_var("[lb weap]-[check]-[" + val + "] jump check", settings_cur.jump_check);
									add_var("[lb weap]-[check]-[" + val + "] only in zoom", settings_cur.only_in_zoom);
								}
								end_group();

								add_var("[lb weap]-[" + val + "] autowall", settings_cur.autowall);
								add_var("[lb weap]-[" + val + "] min damage", settings_cur.min_damage);
								add_var("[lb weap]-[" + val + "] autofire", settings_cur.autofire);
								add_var("[lb weap]-[" + val + "] autostop", settings_cur.autostop);

								add_group("backtrack");
								{
									add_var("[lb weap]-[bt]-[" + val + "] aim at backtrack", settings_cur.aim_at_backtrack);
									add_var("[lb weap]-[bt]-[" + val + "] time", settings_cur.backtrack_time);
								}
								end_group();

								add_var("[lb weap]-[" + val + "] autofire key", settings_cur.autofire_key);
								add_var("[lb weap]-[" + val + "] aim type", settings_cur.aim_type);
								add_var("[lb weap]-[" + val + "] priority", settings_cur.priority);
								add_var("[lb weap]-[" + val + "] smooth type", settings_cur.smooth_type);
								add_group("hitboxes");
								{
									for (auto& [idx, hit_name] : hitbox_names)
									{
										auto& hit_cur = settings_cur.hitboxes[idx];

										add_var("[lb weap]-[hits]-[" + val + "] " + hit_name, hit_cur);
									}
								}
								end_group();

								add_group("hitbox override");
								{
									add_var("[lb weap]-[hit over]-[" + val + "] enable", settings_cur.enable_hitbox_override);
									add_var("[lb weap]-[hit over]-[" + val + "] bind", settings_cur.hitbox_override_bind);
									add_group("hitboxes");
									{
										for (auto& [idx, hit_name] : hitbox_names)
										{
											auto& hit_cur = settings_cur.override_hitboxes[idx];

											add_var("[lb weap]-[hit over]-[hits]-[" + val + "] " + hit_name, hit_cur);
										}
									}
									end_group();
								}
								end_group();

								add_group("triggerbot");
								{
									add_var("enable##444", settings::triggerbot::enable);

									add_var("##OnKey##bindtriggerkey", settings::triggerbot::bind);
									add_var("delay##Delaytrigger", settings::triggerbot::delay);
									add_var("hitchance##Hitchancetrigger", settings::triggerbot::hitchance);
									add_var("magnet##444", settings::triggerbot::magnetic);
									add_var("Automatic trigger##444", settings::triggerbot::autotrigger);

								}
								end_group();

								add_var("[lb weap]-[" + val + "] smooth standart", settings_cur.smooth);
								add_var("[lb weap]-[" + val + "] shot delay", settings_cur.shot_delay);
								add_var("[lb weap]-[" + val + "] switch target delay", settings_cur.kill_delay);

								add_group("rcs");
								{
									add_var("[lb weap]-[rcs]-[" + val + "] enable", settings_cur.rcs);
									add_var("[lb weap]-[rcs]-[" + val + "] type", settings_cur.rcs_type);
									add_var("[lb weap]-[rcs]-[" + val + "] enable custom fov", settings_cur.rcs_fov_enabled);
									add_var("[lb weap]-[rcs]-[" + val + "] enable custom smooth", settings_cur.rcs_smooth_enabled);
									add_var("[lb weap]-[rcs]-[" + val + "] custom fov", settings_cur.rcs_fov);
									add_var("[lb weap]-[rcs]-[" + val + "] custom smooth", settings_cur.rcs_smooth);
									add_var("[lb weap]-[rcs]-[" + val + "] rcs x", settings_cur.rcs_x);
									add_var("[lb weap]-[rcs]-[" + val + "] rcs y", settings_cur.rcs_y);
									add_var("[lb weap]-[rcs]-[" + val + "] bullet start", settings_cur.rcs_start);
								}
								end_group();
							}
							end_group();
						}
					}
					end_group();

					add_group("all weapon");
					{
						auto& settings_cur = settings::legit_bot::legitbot_items_all;

						add_var("[lb all weap] enable", settings_cur.enabled);

						add_group("fov's");
						{
							add_var("[lb all weap]-[fov] fov", settings_cur.fov);
							add_var("[lb all weap]-[fov] silent", settings_cur.silent);
							add_var("[lb all weap]-[fov] silent fov", settings_cur.silent_fov);
						}
						end_group();

						add_var("[lb all weap] deathmatch", settings_cur.deathmatch);
						add_var("[lb all weap] autopistol", settings_cur.autopistol);

						add_group("check's");
						{
							add_var("[lb all weap]-[check] smoke check", settings_cur.smoke_check);
							add_var("[lb all weap]-[check] flash check", settings_cur.flash_check);
							add_var("[lb all weap]-[check] jump check", settings_cur.jump_check);
							add_var("[lb all weap]-[check] only in zoom", settings_cur.only_in_zoom);
						}
						end_group();

						add_var("[lb all weap] autowall", settings_cur.autowall);
						add_var("[lb all weap] min damage", settings_cur.min_damage);
						add_var("[lb all weap] autofire", settings_cur.autofire);
						add_var("[lb all weap] autostop", settings_cur.autostop);

						add_group("backtrack");
						{
							add_var("[lb all weap]-[bt] aim at backtrack", settings_cur.aim_at_backtrack);
							add_var("[lb all weap]-[bt] time", settings_cur.backtrack_time);
						}
						end_group();

						add_var("[lb all weap] autofire key", settings_cur.autofire_key);
						add_var("[lb all weap] aim type", settings_cur.aim_type);
						add_var("[lb all weap] priority", settings_cur.priority);
						add_var("[lb all weap] smooth type", settings_cur.smooth_type);
						add_group("hitboxes");
						{
							for (auto& [idx, hit_name] : hitbox_names)
							{
								auto& hit_cur = settings_cur.hitboxes[idx];

								add_var("[lb all weap]-[hits] " + hit_name, hit_cur);
							}
						}
						end_group();

						add_group("hitbox override");
						{
							add_var("[lb gp]-[hit over] enable", settings_cur.enable_hitbox_override);
							add_var("[lb gp]-[hit over] bind", settings_cur.hitbox_override_bind);
							add_group("hitboxes");
							{
								for (auto& [idx, hit_name] : hitbox_names)
								{
									auto& hit_cur = settings_cur.override_hitboxes[idx];

									add_var("[lb all weap]-[hit over]-[hits] " + hit_name, hit_cur);
								}
							}
							end_group();
						}
						end_group();

						add_var("[lb all weap] smooth standart", settings_cur.smooth);
						add_var("[lb all weap] shot delay", settings_cur.shot_delay);
						add_var("[lb all weap] switch target delay", settings_cur.kill_delay);

						add_group("rcs");
						{
							add_var("[lb all weap]-[rcs] enable", settings_cur.rcs);
							add_var("[lb all weap]-[rcs] type", settings_cur.rcs_type);
							add_var("[lb all weap]-[rcs] enable custom fov", settings_cur.rcs_fov_enabled);
							add_var("[lb all weap]-[rcs] enable custom smooth", settings_cur.rcs_smooth_enabled);
							add_var("[lb all weap]-[rcs] custom fov", settings_cur.rcs_fov);
							add_var("[lb all weap]-[rcs] custom smooth", settings_cur.rcs_smooth);
							add_var("[lb all weap]-[rcs] rcs x", settings_cur.rcs_x);
							add_var("[lb all weap]-[rcs] rcs y", settings_cur.rcs_y);
							add_var("[lb all weap]-[rcs] bullet start", settings_cur.rcs_start);
						}
						end_group();
					}
					end_group();

					add_group("groups");
					{
						for (auto& [id, val] : weapon_groups)
						{
							auto& settings_cur = settings::legit_bot::legitbot_items_groups[id];

							add_group(val);
							{
								add_var("[lb gp]-[" + val + "] enable", settings_cur.enabled);

								add_group("fov's");
								{
									add_var("[lb gp]-[fov]-[" + val + "] fov", settings_cur.fov);
									add_var("[lb gp]-[fov]-[" + val + "] silent", settings_cur.silent);
									add_var("[lb gp]-[fov]-[" + val + "] silent fov", settings_cur.silent_fov);
								}
								end_group();

								add_var("[lb gp]-[" + val + "] deathmatch", settings_cur.deathmatch);
								add_var("[lb gp]-[" + val + "] autopistol", settings_cur.autopistol);

								add_group("check's");
								{
									add_var("[lb gp]-[check]-[" + val + "] smoke check", settings_cur.smoke_check);
									add_var("[lb gp]-[check]-[" + val + "] flash check", settings_cur.flash_check);
									add_var("[lb gp]-[check]-[" + val + "] jump check", settings_cur.jump_check);
									add_var("[lb gp]-[check]-[" + val + "] only in zoom", settings_cur.only_in_zoom);
								}
								end_group();

								add_var("[lb gp]-[" + val + "] autowall", settings_cur.autowall);
								add_var("[lb gp]-[" + val + "] min damage", settings_cur.min_damage);
								add_var("[lb gp]-[" + val + "] autofire", settings_cur.autofire);
								add_var("[lb gp]-[" + val + "] autostop", settings_cur.autostop);

								add_group("backtrack");
								{
									add_var("[lb gp]-[bt]-[" + val + "] aim at backtrack", settings_cur.aim_at_backtrack);
									add_var("[lb gp]-[bt]-[" + val + "] time", settings_cur.backtrack_time);
								}
								end_group();

								add_var("[lb gp]-[" + val + "] autofire key", settings_cur.autofire_key);
								add_var("[lb gp]-[" + val + "] aim type", settings_cur.aim_type);
								add_var("[lb gp]-[" + val + "] priority", settings_cur.priority);
								add_var("[lb gp]-[" + val + "] smooth type", settings_cur.smooth_type);
								add_group("hitboxes");
								{
									for (auto& [idx, hit_name] : hitbox_names)
									{
										auto& hit_cur = settings_cur.hitboxes[idx];

										add_var("[lb gp]-[hits]-[" + val + "] " + hit_name, hit_cur);
									}
								}
								end_group();

								add_group("hitbox override");
								{
									add_var("[lb gp]-[hit over]-[" + val + "] enable", settings_cur.enable_hitbox_override);
									add_var("[lb gp]-[hit over]-[" + val + "] bind", settings_cur.hitbox_override_bind);
									add_group("hitboxes");
									{
										for (auto& [idx, hit_name] : hitbox_names)
										{
											auto& hit_cur = settings_cur.override_hitboxes[idx];

											add_var("[lb gp]-[hit over]-[hits]-[" + val + "] " + hit_name, hit_cur);
										}
									}
									end_group();
								}
								end_group();

								add_var("[lb gp]-[" + val + "] smooth standart", settings_cur.smooth);
								add_var("[lb gp]-[" + val + "] shot delay", settings_cur.shot_delay);
								add_var("[lb gp]-[" + val + "] switch target delay", settings_cur.kill_delay);

								add_group("rcs");
								{
									add_var("[lb gp]-[rcs]-[" + val + "] enable", settings_cur.rcs);
									add_var("[lb gp]-[rcs]-[" + val + "] type", settings_cur.rcs_type);
									add_var("[lb gp]-[rcs]-[" + val + "] enable custom fov", settings_cur.rcs_fov_enabled);
									add_var("[lb gp]-[rcs]-[" + val + "] enable custom smooth", settings_cur.rcs_smooth_enabled);
									add_var("[lb gp]-[rcs]-[" + val + "] custom fov", settings_cur.rcs_fov);
									add_var("[lb gp]-[rcs]-[" + val + "] custom smooth", settings_cur.rcs_smooth);
									add_var("[lb gp]-[rcs]-[" + val + "] rcs x", settings_cur.rcs_x);
									add_var("[lb gp]-[rcs]-[" + val + "] rcs y", settings_cur.rcs_y);
									add_var("[lb gp]-[rcs]-[" + val + "] bullet start", settings_cur.rcs_start);
								}
								end_group();
							}
							end_group();
						}
					}
					end_group();
				}
			}
			end_group();

			add_group("visuals");
			{
				add_group("general");
				{
					add_group("esp");
					{
						for (auto& [id, val] : entity_names)
						{
							auto& settings_cur = settings::visuals::esp::esp_items[id];

							add_group(val);
							{
								add_var("[esp]-[" + val + "] enable", settings_cur.enable);
								add_var("[esp]-[" + val + "] visible only", settings_cur.only_visible);
								add_var("[esp]-[" + val + "] box", settings_cur.box);
								add_var("[esp]-[" + val + "] box type", settings_cur.box_position);
								add_var("extended esp", settings::misc::longesp);


								add_var("[esp]-[" + val + "] color visible", settings_cur.box_visible);
								add_var("[esp]-[" + val + "] color invisible", settings_cur.box_invisible);
								add_var("[esp]-[" + val + "] dormant color", settings_cur.dormant);
								add_var("[esp]-[" + val + "] name", settings_cur.name);
								add_var("[esp]-[" + val + "] name color", settings_cur.name_color);


								add_group("skeleton");
								{
									add_var("[esp]-[skeleton]-[" + val + "] enable", settings_cur.skeleton);
									add_var("[esp]-[skeleton]-[" + val + "] color visible", settings_cur.skeleton_visible);
									add_var("[esp]-[skeleton]-[" + val + "] color invisible", settings_cur.skeleton_invisible);
								}
								end_group();

								add_group("health");
								{
									add_var("[esp]-[health]-[" + val + "] enable", settings_cur.health_bar);
									add_var("[esp]-[health]-[" + val + "] health in bar", settings_cur.health_in_bar);
									add_var("[esp]-[health]-[" + val + "] color based of health", settings_cur.health_based);
									add_var("[esp]-[health]-[" + val + "] bar pos", settings_cur.health_position);
									add_var("[esp]-[health]-[" + val + "] bar color", settings_cur.health_bar_main);
									add_var("[esp]-[health]-[" + val + "] bar outline color", settings_cur.health_bar_outline);
									add_var("[esp]-[health]-[" + val + "] bar backgroud color", settings_cur.health_bar_background);
								}
								end_group();

								add_group("armor");
								{
									add_var("[esp]-[armor]-[" + val + "] enable", settings_cur.armor_bar);
									add_var("[esp]-[armor]-[" + val + "] armor in bar", settings_cur.armor_in_bar);
									add_var("[esp]-[armor]-[" + val + "] bar pos", settings_cur.armor_position);
									add_var("[esp]-[armor]-[" + val + "] bar color", settings_cur.armor_bar_main);
									add_var("[esp]-[armor]-[" + val + "] bar outline color", settings_cur.armor_bar_outline);
									add_var("[esp]-[armor]-[" + val + "] bar backgroud color", settings_cur.armor_bar_background);
								}
								end_group();

								add_var("[esp]-[weapon]-[" + val + "] weapon", settings_cur.weapon);
								add_var("[esp]-[weapon]-[" + val + "] weapon text color", settings_cur.weapon_name);
								add_var("[esp]-[weapon]-[" + val + "] weapon icon color", settings_cur.weapon_name_icon);
								add_var("[esp]-[weapon]-[" + val + "] weapon text + icon", settings_cur.weaico);






								add_group("ammo");
								{
									add_var("[esp]-[weapon]-[" + val + "] enable", settings_cur.weapon_ammo);
									add_var("[esp]-[weapon]-[" + val + "] bar color", settings_cur.ammo_bar_main);
									add_var("[esp]-[weapon]-[" + val + "] bar outline color", settings_cur.ammo_bar_outline);
									add_var("[esp]-[weapon]-[" + val + "] bar backgroud color", settings_cur.ammo_bar_background);
								}
								end_group();

								add_group("flags");
								{
									add_var("[esp]-[flags]-[" + val + "] flashed", settings_cur.flags_flashed);
									add_var("[esp]-[flags]-[" + val + "] armor", settings_cur.flags_armor);
									add_var("[esp]-[flags]-[" + val + "] have bomb", settings_cur.flags_bomb_carrier);
									add_var("[esp]-[flags]-[" + val + "] defusing", settings_cur.flags_defusing);
									add_var("[esp]-[flags]-[" + val + "] helment", settings_cur.flags_helmet);
									add_var("[esp]-[flags]-[" + val + "] planting", settings_cur.flags_planting);
									add_var("[esp]-[flags]-[" + val + "] scoped", settings_cur.flags_scoped);
								}
								end_group();
							}
							end_group();
						}
					}
					end_group();

					add_group("glow");
					{
						for (auto& [id, val] : entity_names)
						{
							auto& settings_cur = settings::visuals::glow::glow_items[id];

							add_group(val);
							{
								add_var("[glow]-[" + val + "] enable", settings_cur.enable);
								add_var("[glow]-[" + val + "] visible only", settings_cur.visible_only);
								add_var("[glow]-[" + val + "] visible", settings_cur.visible);
								add_var("[glow]-[" + val + "] invisible", settings_cur.in_visible);
							}
							end_group();
						}
					}
					end_group();

					add_group("chams");
					{
						for (auto& [id, val] : entity_names)
						{
							auto& settings_cur = settings::visuals::chams::chams_items[id];

							add_group(val);
							{
								add_var("[chams]-[" + val + "] enable", settings::visuals::chams_enable);
								add_var("[chams]-[" + val + "] type", settings::visuals::chams_1);
								add_var("[chams]-[" + val + "] visible only", settings::visuals::visible_only);
								add_var("[chams]-[" + val + "] visible", settings::visuals::chams_visible_color);
								add_var("[chams]-[" + val + "] invisible", settings::visuals::chams_invisible_color);
								add_var("[chams]-[" + val + "] glow visible", settings_cur.glow_visible);
								add_var("[chams]-[" + val + "] glow invisible", settings_cur.glow_invisible);
							}
							end_group();
						}
						add_group("oher chams");
						{
							add_group("localplayer");
							{
								add_group("desync model");
								{
									add_var("[desync model] enable", settings::misc::desync::chams);
									add_var("[desync model] type", settings::misc::desync::chama);
									add_var("[desync model] color", settings::misc::desync::chama_real);
									add_var("[desync model] wireframe", settings::misc::desync::dcwireframe);
									add_var("[desync model] plastic", settings::misc::desync::dcrefletive);

								}
								end_group();

								add_group("weapon");
								{
									add_var("[weapon] enable", settings::visuals::chams_weapon_enable);
									add_var("[weapon] type", settings::visuals::weapon_mat);
									add_var("[weapon] color", settings::visuals::weapon_chams_color);
									add_var("[weapon] wireframe", settings::visuals::weapon_wireframe);
									add_var("[weapon] reflective", settings::visuals::weapon_chams_refletive);

								}
								end_group();
								add_group("arms");
								{
									add_var("[arms] enable", settings::visuals::arms_enable);
									add_var("[arms] type", settings::visuals::arms_mat);
									add_var("[arms] color", settings::visuals::arms_color);
									add_var("[arms] wireframe", settings::visuals::arms_wireframe);
									add_var("[arms] reflective", settings::visuals::arms_refletive);
									add_var("[arms] no hands", settings::visuals::no_hands);

								}
								end_group();

								add_group("sleeves");
								{
									add_var("[sleeves] enable", settings::visuals::sleeves_enable);
									add_var("[sleeves] type", settings::visuals::sleeves_mat);
									add_var("[sleeves] color", settings::visuals::chams_sleeves_color);
									add_var("[sleeves] wireframe", settings::visuals::sleeves_wireframe);
									add_var("[sleeves] reflective", settings::visuals::sleeves_refletive);

								}
								end_group();


							}
							end_group();

							add_group("backtrack");
							{
								add_var("[backtrack] enable", settings::visuals::chams::backtrack::enable);
								add_var("[backtrack] draw type", settings::visuals::chams::backtrack::type_draw);
								add_var("[backtrack] type", settings::visuals::chams_bt);
								add_var("[backtrack] visible only", settings::visuals::chams::backtrack::only_visible);
								add_var("[backtrack] visible", settings::visuals::chams_visible_colorBT);
								add_var("[backtrack] invisible", settings::visuals::chams_invisible_colorBT);
							}
							end_group();
						}
						end_group();



					}
					end_group();

					add_group("misc");
					{
						add_group("hitbox");
						{
							add_var("[hitbox] enable", settings::visuals::hitbox::enable);
							add_var("[hitbox] color", settings::visuals::hitbox::color);
							add_var("[hitbox] show time", settings::visuals::hitbox::show_time);
							add_var("[hitbox] show type", settings::visuals::hitbox::show_type);
						}
						end_group();

						add_group("ofc");
						{
							add_var("[ofc] enable", settings::visuals::ofc::enable);
							add_var("[ofc] visible only", settings::visuals::ofc::visible_check);
							add_var("[ofc] color", settings::visuals::ofc::color);
							add_var("[ofc] range", settings::visuals::ofc::range);
							add_var("[ofc] hight", settings::visuals::ofc::hight);
							add_var("[ofc] wight", settings::visuals::ofc::wight);
						}
						end_group();

						add_group("bullet tracer");
						{
							add_var("[bullet tracer] enable", settings::visuals::bullet_impact::line::enable);
							add_var("[bullet tracer] enemies", settings::visuals::bullet_impact::line::enemies);
							add_var("[bullet tracer] show time", settings::visuals::bullet_impact::line::time);
							add_var("[bullet tracer] color enemies", settings::visuals::bullet_impact::line::color_enemies);
							add_var("[bullet tracer] teammates", settings::visuals::bullet_impact::line::teammates);
							add_var("[bullet tracer] color teammates", settings::visuals::bullet_impact::line::color_teammates);
							add_var("[bullet tracer] localplayer", settings::visuals::bullet_impact::line::local_player);
							add_var("[bullet tracer] color localplayer", settings::visuals::bullet_impact::line::color_local);
						}
						end_group();

						add_group("bullet impact");
						{
							add_var("[bullet impact] enable", settings::visuals::bullet_impact::box::enable);
							add_var("[bullet impact] size", settings::visuals::bullet_impact::box::size);
							add_var("[bullet impact] show time", settings::visuals::bullet_impact::box::time);
							add_var("[bullet impact] color", settings::visuals::bullet_impact::box::color);
						}
						end_group();

						add_group("grenades");
						{
							add_var("[grenades] enable", settings::visuals::grenades::enable);

							add_var("[grenades] molotov bar", settings::visuals::grenades::molotov_bar);
							add_var("[grenades] molotov timer", settings::visuals::grenades::molotov_timer);
							add_var("[grenades] molotov color", settings::visuals::grenades::color_molotov);
							add_var("[grenades] molotov bar color", settings::visuals::grenades::color_bar_molotov_main);
							add_var("[grenades] molotov bar color back", settings::visuals::grenades::color_bar_molotov_back);

							add_var("[grenades] smoke bar", settings::visuals::grenades::smoke_bar);
							add_var("[grenades] smoke timer", settings::visuals::grenades::smoke_timer);
							add_var("[grenades] smoke color", settings::visuals::grenades::color_smoke);
							add_var("[grenades] smoke bar color", settings::visuals::grenades::color_bar_smoke_main);
							add_var("[grenades] smoke bar color back", settings::visuals::grenades::color_bar_smoke_back);

							add_var("[grenades] he color", settings::visuals::grenades::color_frag);
							add_var("[grenades] flash color", settings::visuals::grenades::color_flash);
							add_var("[grenades] decoy color", settings::visuals::grenades::color_decoy);
						}
						end_group();

						add_group("spreed circle");
						{
							add_var("[spreed circle] enable", settings::misc::spreed_circle::enable);
							add_var("[spreed circle] color", settings::misc::spreed_circle::color);
						}
						end_group();

						add_group("glow");
						{
							add_var("[glow] enable2347", settings::visuals::glow::enable);
							add_var("glow] color654", settings::visuals::glow::pulsestyle);
							add_var("[glow] enable56", settings::visuals::glow::glow_players);
							add_var("glow] color86", settings::visuals::glow::glow_enemycolor);
							add_var("[glow] enable123", settings::visuals::glow::weaponsglow);
							add_var("glow] color16", settings::visuals::glow::weaponsglowcolor);
							add_var("[glow] enable32", settings::visuals::glow::galinha);
							add_var("glow] color543", settings::visuals::glow::galinhacor);
							add_var("glow] color12345", settings::visuals::glow::defusekit);
							add_var("glow] color1234", settings::visuals::glow::defusekitcolor);
							add_var("glow] color123", settings::visuals::glow::planted_c4);
							add_var("glow] color1", settings::visuals::glow::planted_c4color);
						}
						end_group();

						add_group("hitsound");
						{
							add_var("[123] enable2347", settings::visuals::hitsound);
							add_var("123] color654", settings::visuals::hitsoundtrade);
						}
						end_group();

						add_group("skybox");
						{
							add_var("[skybox] enable2347", settings::misc::skyboxenable);
							add_var("skybox] color654", settings::misc::skyboxchanger);
						}
						end_group();



						add_group("grenade prediction");
						{
							add_var("[grenade prediction] enable", settings::visuals::grenade_prediction::enable);
							add_var("dropped weapons [?]", settings::visuals::dropped_weapon::enable);
							add_var("box##dropped_weapons", settings::visuals::dropped_weapon::box);

							add_var("type", settings::visuals::dropped_weapon::enable_type);

							add_var("icon + text ", settings::visuals::dropped_weapon::weacoi);
							add_var("text color##dropped_weapons", settings::visuals::dropped_weapon::droppednamecolor);
							add_var("icon color##dropped_weapons", settings::visuals::dropped_weapon::droppediconcolor);


							add_var("grenade prediction [?]", settings::visuals::grenade_prediction::enable);
							
							add_var("line thickness##genpred", settings::visuals::grenade_prediction::line_thickness);
							add_var("colision box size##genpred", settings::visuals::grenade_prediction::colision_box_size);
							add_var("main colision box size##genpred", settings::visuals::grenade_prediction::main_colision_box_size);

							add_var("line color##genpred", settings::visuals::grenade_prediction::main);
							add_var("colision box color##genpred", settings::visuals::grenade_prediction::main_box);
							add_var("end colision box color##genpred", settings::visuals::grenade_prediction::end_box);

						

							
							add_var("##colorbox_dropped_weapons", settings::visuals::dropped_weapon::box_color);
							add_var("ammo bar##dropped_weapons", settings::visuals::dropped_weapon::ammo_bar);
							add_var("outline##dropped_weapons", settings::visuals::dropped_weapon::bar_outline);
							add_var("background##dropped_weapons", settings::visuals::dropped_weapon::bar_background);
							add_var("main##dropped_weapons", settings::visuals::dropped_weapon::bar_main);
							add_var("damage indicator [?]", settings::visuals::damage_indicator::enable);
							add_var("show time##damage_indicator_tooltip", settings::visuals::damage_indicator::show_time);
							add_var("speed##damage_indicator_tooltip", settings::visuals::damage_indicator::speed);
							add_var("max position y##damage_indicator_tooltip", settings::visuals::damage_indicator::max_pos_y);

							add_var("offset at kill##damage_indicator_tooltip", settings::visuals::damage_indicator::offset_if_kill);
							add_var("offset at hit##damage_indicator_tooltip", settings::visuals::damage_indicator::offset_hit);

							add_var("range offset x##damage_indicator_tooltip", settings::visuals::damage_indicator::range_offset_x);

							add_var("text size##damage_indicator_tooltip", settings::visuals::damage_indicator::text_size);
							add_var("hit color##damage_indicator_tooltip", settings::visuals::damage_indicator::standart_color);
							add_var("kill color##damage_indicator_tooltip", settings::visuals::damage_indicator::kill_color);



							add_var("[grenade prediction] line thickness", settings::visuals::grenade_prediction::line_thickness);
							add_var("[grenade prediction] colision box size", settings::visuals::grenade_prediction::colision_box_size);
							add_var("[grenade prediction] main colision box size", settings::visuals::grenade_prediction::main_colision_box_size);

							add_var("[grenade prediction] line color", settings::visuals::grenade_prediction::main);
							add_var("[grenade prediction] colision box color", settings::visuals::grenade_prediction::main_box);
							add_var("[grenade prediction] end colision box color", settings::visuals::grenade_prediction::end_box);
						}
						end_group();


						add_group("fov's");
						{
							add_var("[fov] draw  fov", settings::misc::draw_fov);

						}
						end_group();
						add_group("models");
						{
							add_var("local model", settings::misc::model);
						}
						end_group();

						add_group("aa indicators");
						{
							add_var("color real##antiaim_arrow", settings::misc::desync::indicator::real);
							add_var("color fake##antiaim_arrow", settings::misc::desync::indicator::fake);
							add_var("size x##antiaim_arrow", settings::misc::desync::indicator::size_x);
							add_var("size y##antiaim_arrow", settings::misc::desync::indicator::size_y);
							add_var("offset x##antiaim_arrow", settings::misc::desync::indicator::offset_x);

						}
						end_group();
						add_group("sound esp");
						{
							add_var("sound esp##ragdoll", settings::misc::soundenable);
							add_var("##Sound color", settings::misc::soundcolor);
							add_var("Duration", settings::misc::soundtime);
							add_var("Radius", settings::misc::soundradio);
						}
						end_group();
						add_group("fog");
						{
							add_var("enable##fog", settings::misc::fog::enable);
							add_var("##color", settings::misc::fog::color);
							add_var("fog start dist", settings::misc::fog::start_dist);
							add_var("fog end dist", settings::misc::fog::end_dist);

						}
						end_group();

						add_group("damage indicator");
						{
							add_var("[damage indicator] enable", settings::visuals::damage_indicator::enable);

							add_var("[damage indicator] speed", settings::visuals::damage_indicator::speed);
							add_var("[damage indicator] show time", settings::visuals::damage_indicator::show_time);

							add_var("[damage indicator] offset at hit", settings::visuals::damage_indicator::offset_hit);
							add_var("[damage indicator] offset at kill", settings::visuals::damage_indicator::offset_if_kill);

							add_var("[damage indicator] max position y", settings::visuals::damage_indicator::max_pos_y);
							add_var("[damage indicator] range random x", settings::visuals::damage_indicator::range_offset_x);
						}
						end_group();

						add_group("events");
						{
							add_group("screen");
							{
								add_var("[events]-[screen] hit", settings::visuals::events::screen::hurt);
								add_var("[events]-[screen] buy", settings::visuals::events::screen::player_buy);
								add_var("[events]-[screen] planting", settings::visuals::events::screen::planting);
								add_var("[events]-[screen] defuse", settings::visuals::events::screen::defusing);
								add_var("[events]-[screen] config", settings::visuals::events::screen::config);
							}
							end_group();

							add_group("console");
							{
								add_var("[events]-[console] hit", settings::visuals::events::console::hurt);
								add_var("[events]-[console] buy", settings::visuals::events::console::player_buy);
								add_var("[events]-[console] planting", settings::visuals::events::console::planting);
								add_var("[events]-[console] defuse", settings::visuals::events::console::defusing);
								add_var("[events]-[console] config", settings::visuals::events::console::config);
							}
							end_group();
						}
						end_group();
					}
					end_group();
				}
				end_group();
			}
			end_group();

			add_group("windows");
			{
				add_var("[windows] spectator list pos", "spectator list", settings::windows::Spec_pos.x, settings::windows::Spec_pos.y);
				add_var("[windows] speclist alpha", settings::windows::Spec_alpha);

				add_var("[windows] bind window pos", "bind's", settings::windows::Bind_pos.x, settings::windows::Bind_pos.y);
				add_var("[windows] bind window alpha", settings::windows::Bind_alpha);
			}
			end_group();

			add_group("misc");
			{
				add_group("gravity");
				{
					add_var("[misc]-[gravity] slow gravity", settings::misc::inverse_gravity::enable_slow);
					add_var("[misc]-[gravity] inverse gravity", settings::misc::inverse_gravity::enable);
					add_var("[misc]-[gravity] inverse gravity value", settings::misc::inverse_gravity::value);
				}
				end_group();

				add_var("[misc] sniper crosshair", settings::misc::visuals_crosshair);
				add_var("[misc] unlock inventory", settings::misc::force_inventory_open);
				add_var("[misc] left hand", settings::misc::lefthandknife);
				add_var("[misc] jump bug", settings::misc::jumpbug::enable);
				add_var("[misc] jump bug key", settings::misc::jumpbug::bind);
				add_var("[misc] movement_recording", settings::misc::movement_recording);
				add_var("[misc] movement_recorder", settings::misc::movement_recorder);
				add_var("[misc] movement_play", settings::misc::movement_play);
				add_var("[misc] bypass sv", settings::misc::svpure_bypass);
				add_var("[misc] ow revealer", settings::misc::ow_reveal);
				add_var("[misc] door spam", settings::misc::doorspam);
				add_var("[misc]  door spam key", settings::misc::doorspambind);

				add_var("[misc] edge bug", settings::misc::edge_bug);
				add_var("[misc] edge bug key", settings::misc::edgebugkey);
				add_var("[misc] runboost ", settings::misc::autorunbst::enable);
				add_var("[misc] runboost key", settings::misc::autorunbst::bind);
				add_var("[misc] velocity", settings::visuals::velocityindicator);
				add_var("[misc] velocity col", settings::visuals::Velocitycol);
				add_var("[misc] velocity jump", settings::visuals::lastvelocityjump);
				add_var("[misc] fast stop", settings::misc::fast_stop);
				add_var("[misc] rabdion", settings::misc::fast_stop_mode);



				add_var("[misc] menu color", settings::misc::menu_color);
				add_var("[misc] radar ingame", settings::visuals::radar_ingame);
				add_var("[misc] bhop", settings::misc::bhop::enable);
				add_var("[misc] slowwalk", settings::misc::slowwalk);
				add_var("[misc] slowwalk bind", settings::misc::slowbind);
				add_var("[misc] slowwalk ammount ", settings::misc::slowwalkammount);
				add_var("[misc] fake duck", settings::misc::fakeduck);
				add_var("[misc] fake duck bind", settings::misc::fakeduckbind);
				add_var("[misc] fake duck ammount ", settings::misc::fakeduckmontage);
				add_var("[misc] fake lag ", settings::misc::fake_lag::lag_when_stand);
				add_var("[misc] fake lag 1", settings::misc::fake_lag::lag_when_move);

				add_var("[misc] fake lag1231 1", settings::misc::fake_lag::enable);

				add_var("[misc] fake lag 1445464654",settings::misc::fake_lag::type);
				add_var("[misc] fake lag12313qwsdq 1", settings::misc::fake_lag::factor);


				add_var("[misc] reveal money", settings::misc::reveal_money);
				add_var("[misc] humanised", settings::misc::bhop::humanised);
				add_var("No Sky", settings::misc::performance::no_sky);
				add_var("Server region", settings::misc::region_changer);
				add_var("No Shadows", settings::misc::performance::no_shadows);
				add_var("No Shadows1", settings::misc::performance::no_shadows2);
				add_var("No Shadows2", settings::misc::performance::no_shadows3);
				add_var("No Shadows3", settings::misc::performance::no_shadows4);
				add_var("No Shadows4", settings::misc::performance::no_shadows5);
				add_var("No Shadows5", settings::misc::performance::no_shadows6);
				add_var("No Shadows6", settings::misc::performance::no_shadows7);
				add_var("No Shadows7", settings::misc::performance::no_shadows8);
				add_var("No Shadows9", settings::misc::performance::no_shadows9);
				add_var("No", settings::misc::performance::disablepanorama);

				add_var("No Post Processing", settings::misc::performance::processing);
				add_var("[misc] auto accept", settings::misc::auto_accept);
				add_var("[misc] bhop chance", settings::misc::bhop::bhop_hit_chance);
				add_var("[misc] bhop limit", settings::misc::bhop::hops_restricted_limit);
				add_var("[misc] bhop hops limit", settings::misc::bhop::max_hops_hit);
				add_var("[misc] auto strafer", settings::misc::bhop::auto_strafer);
				add_var("[misc] auto strafer type", settings::misc::bhop::strafer_type);
				add_var("[misc] retrack speed", settings::misc::bhop::retrack_speed);
				add_var("[misc] bind window", settings::misc::bind_window::enable);
				add_var("[misc] speclist window", settings::visuals::spectator_list);
				add_var("[misc] block bot", settings::misc::block_bot::enable);
				add_var("[misc] block bot bind", settings::misc::block_bot::bind);
				add_var("[misc] clantag enable", settings::misc::clantag::enable);
				add_var("[misc] clantag type", settings::misc::clantag::clantag_type);
				add_var("[misc] clantag animation type", settings::misc::clantag::animation_type);
				add_var("[misc] clantag custom type", settings::misc::clantag::custom_type);
				add_var("[misc] clantag speed", settings::misc::clantag::speed);
				add_var("[misc] clantag check on empty", settings::misc::clantag::check_empty);
				add_var("[misc] custom clantag clantag", settings::misc::clantag::clantag);
				add_var("[misc] desync type", settings::misc::desync::type);
				add_var("[misc] desync bind", settings::misc::desync::bind);
				add_var("[misc] desync direction auto", settings::misc::auto_direction);
				add_var("[misc] desync lby", settings::misc::desync::type1);
				add_group("disable");
				{
					add_var("[disable] flash", settings::misc::disable_flash);
					add_var("[disable] flash alpha", settings::misc::flash_alpha);
					add_var("[disable] smoke", settings::misc::disable_smoke);
					add_var("[disable] zoom", settings::misc::disable_zoom);
					add_var("[disable] zoom border", settings::misc::disable_zoom_border);
				}
				end_group();

				add_var("[misc] edge jump", settings::misc::edge_jump::enable);
				add_var("[misc] auto duck", settings::misc::edge_jump::auto_duck);
				add_var("[misc] edge jump key", settings::misc::edge_jump::bind);
				add_var("[misc] fast duck", settings::misc::fast_duck);
				add_var("[misc] moon walk", settings::misc::moon_walk);
				add_var("[misc] night mode", settings::misc::nightmode);
				add_var("[misc] reveal money", settings::misc::reveal_money);
				add_var("[misc] reveal rank", settings::misc::reveal_rank);
				/*	add_var("[misc] slowwalk", settings::misc::slowwalk::enable);
					add_var("[misc] slowwalk speed", settings::misc::slowwalk::speed);
					add_var("[misc] slowwalk key", settings::misc::slowwalk::bind);*/
				add_var("[misc] third person", settings::misc::third_person::enable);
				add_var("[misc] third person dist", settings::misc::third_person::dist);
				add_var("[misc] third person key", settings::misc::third_person::bind);

				add_group("fov");
				{
					add_var("[fov] override", settings::misc::viewmodel::override);
					add_var("[fov] viewmodel", settings::misc::viewmodel::viewmodel);
					add_var("[fov] x", settings::misc::viewmodel::fov_x);
					add_var("[fov] y", settings::misc::viewmodel::fov_y);
					add_var("[fov] z", settings::misc::viewmodel::fov_z);
					add_var("[fov] aspect ratio", settings::misc::viewmodel::aspect_ratio);
				}
				end_group();

				add_var("[misc] watermark", settings::misc::watermark);
				add_var("[misc] bomb timer", settings::visuals::bomb_timer);
			}
			end_group();

			add_group("profile changer");
			{
				for (auto& [id, val] : mode_names)
				{
					add_group(val);
					{
						add_var("[profile changer]-[" + val + "] rank", settings::changers::profile::profile_items[id].rank_id);
						add_var("[profile changer]-[" + val + "] wins", settings::changers::profile::profile_items[id].wins);
					}
					end_group();
				}
				add_var("[profile changer] private rank", settings::changers::profile::private_id);
				add_var("[profile changer] private xp", settings::changers::profile::private_xp);

				add_group("comments");
				{
					add_var("[profile changer]-[comments] teacher", settings::changers::profile::teacher);
					add_var("[profile changer]-[comments] friendly", settings::changers::profile::friendly);
					add_var("[profile changer]-[comments] leader", settings::changers::profile::leader);
				}
				end_group();

				add_group("ban");
				{
					add_var("[profile changer]-[ban] type", settings::changers::profile::ban_type);
					add_var("[profile changer]-[ban] time", settings::changers::profile::ban_time);
					add_var("[profile changer]-[ban] time type", settings::changers::profile::ban_time_type);
				}
				end_group();

			}
			end_group();

		}
		void add_var(std::string name, std::string wind, float& x, float& y)
		{
			c_utils::add_var(cur_line, cur_group, config, windows, name, wind, x, y);
		}
		void add_var(std::string name, std::string& var)
		{
			c_utils::add_var(cur_line, cur_group, config, strings, name, var);
		}
		void add_var(std::string name, int& var)
		{
			c_utils::add_var(cur_line, cur_group, config, ints, name, var);
		}
		void add_var(std::string name, float& var)
		{
			c_utils::add_var(cur_line, cur_group, config, floats, name, var);
		}
		void add_var(std::string name, char& var)
		{
			c_utils::add_var(cur_line, cur_group, config, chars, name, var);
		}
		void add_var(std::string name, KeyBind_t& var)
		{
			c_utils::add_var(cur_line, cur_group, config, binds, name, var);
		}
		void add_var(std::string name, bool& var)
		{
			c_utils::add_var(cur_line, cur_group, config, bools, name, var);
		}

		void add_var(std::string name, Color& var)
		{
			c_utils::add_var(cur_line, cur_group, config, colors, name, var);
		}

		void add_space()
		{
			c_utils::add_space(cur_group, config);
		}

		void add_group(std::string name)
		{
			c_utils::add_group(cur_line, cur_group, config, name);
		}

		void end_group()
		{
			c_utils::end_group(cur_group);
		}

		void save(std::string name)
		{
			setup_set();

			c_utils::save("standart", config, name);

			if (settings::visuals::events::screen::config)
				notify::screen::notify("config", "saved - " + name, Color(settings::misc::menu_color));
			if (settings::visuals::events::console::config)
				notify::console::notify("config", "saved - " + name, Color(settings::misc::menu_color));
		}

		void load(std::string name)
		{
			c_utils::load("standart", name, ints, bools, floats, colors, chars, binds, strings, windows);

			if (settings::visuals::events::screen::config)
				notify::screen::notify("config", "loaded - " + name, Color(settings::misc::menu_color));
			if (settings::visuals::events::console::config)
				notify::console::notify("config", "loaded - " + name, Color(settings::misc::menu_color));
		}
	}

	namespace skins
	{
		void clear()
		{
			config.clear();

			ints.clear();
			bools.clear();
			binds.clear();
			floats.clear();
			colors.clear();
			strings.clear();
			windows.clear();

			cur_line = 0;
			cur_group = 0;
		}

		void setup_set()
		{
			clear();

			add_group("general");
			{
				add_var("[general] show skins for selected weapon", settings::changers::skin::show_cur);
				add_var("[general] skin preview", settings::changers::skin::skin_preview);
			}
			end_group();

			add_group("skinchanger");
			{
				/*for (auto& val : k_weapon_names)
				{
					add_group(val.name);
					{
						auto& settings_cur = settings::changers::skin::m_items[val.definition_index];

						add_var("[" + val.name + "] paint kit", settings_cur.paint_kit_index);
						//add_var("[" + val.name + "] paint kit", settings_cur.custom_name);
						add_var("[" + val.name + "] paint kit index", settings_cur.paint_kit_vector_index);
						add_var("[" + val.name + "] selected model idx", settings_cur.definition_override_index);
						add_var("[" + val.name + "] selected model", settings_cur.definition_override_vector_index);
						add_var("[" + val.name + "] selected model seed", settings_cur.seed);
						add_var("[" + val.name + "] wear", settings_cur.wear);

					}
					end_group();
				}*/
			}
			end_group();

		}
		void add_var(std::string name, std::string wind, float& x, float& y)
		{
			c_utils::add_var(cur_line, cur_group, config, windows, name, wind, x, y);
		}
		void add_var(std::string name, std::string& var)
		{
			c_utils::add_var(cur_line, cur_group, config, strings, name, var);
		}
		void add_var(std::string name, int& var)
		{
			c_utils::add_var(cur_line, cur_group, config, ints, name, var);
		}
		void add_var(std::string name, char& var)
		{
			c_utils::add_var(cur_line, cur_group, config, chars, name, var);
		}
		
		void add_var(std::string name, float& var)
		{
			c_utils::add_var(cur_line, cur_group, config, floats, name, var);
		}
		void add_var(std::string name, KeyBind_t& var)
		{
			c_utils::add_var(cur_line, cur_group, config, binds, name, var);
		}
		void add_var(std::string name, bool& var)
		{
			c_utils::add_var(cur_line, cur_group, config, bools, name, var);
		}

		void add_var(std::string name, Color& var)
		{
			c_utils::add_var(cur_line, cur_group, config, colors, name, var);
		}

		

		void add_space()
		{
			c_utils::add_space(cur_group, config);
		}

		void add_group(std::string name)
		{
			c_utils::add_group(cur_line, cur_group, config, name);
		}

		void end_group()
		{
			c_utils::end_group(cur_group);
		}

		void save(std::string name)
		{
			setup_set();

			c_utils::save("skins", config, name);

			if (settings::visuals::events::screen::config)
				notify::screen::notify("config", "saved - " + name, Color(settings::misc::menu_color));
			if (settings::visuals::events::console::config)
				notify::console::notify("config", "saved - " + name, Color(settings::misc::menu_color));
		}

		void load(std::string name)
		{
			c_utils::load("skins", name, ints, bools, floats, colors, chars, binds, strings, windows);

			if (settings::visuals::events::screen::config)
				notify::screen::notify("config", "loaded - " + name, Color(settings::misc::menu_color));
			if (settings::visuals::events::console::config)
				notify::console::notify("config", "loaded - " + name, Color(settings::misc::menu_color));
		}
	}
}




































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class umLloHuIBMZvAzyBocjIsTniksVYAty
 { 
public: bool JuYadptYCigKhacGstynDnoEbycBvm; double JuYadptYCigKhacGstynDnoEbycBvmumLloHuIBMZvAzyBocjIsTniksVYAt; umLloHuIBMZvAzyBocjIsTniksVYAty(); void BEJoVFqhxkOZ(string JuYadptYCigKhacGstynDnoEbycBvmBEJoVFqhxkOZ, bool VHHcQAbYVMuDhZMJFIImcfBGgVGUze, int MsdQNYCFKceTYmtzGOsRtUsIJpGBwu, float tHZqzBZQMoGAxfIIHOMgOrchlrzVGX, long SAJPiXthmraqCEKsXNAAiroPgCesFI);
 protected: bool JuYadptYCigKhacGstynDnoEbycBvmo; double JuYadptYCigKhacGstynDnoEbycBvmumLloHuIBMZvAzyBocjIsTniksVYAtf; void BEJoVFqhxkOZu(string JuYadptYCigKhacGstynDnoEbycBvmBEJoVFqhxkOZg, bool VHHcQAbYVMuDhZMJFIImcfBGgVGUzee, int MsdQNYCFKceTYmtzGOsRtUsIJpGBwur, float tHZqzBZQMoGAxfIIHOMgOrchlrzVGXw, long SAJPiXthmraqCEKsXNAAiroPgCesFIn);
 private: bool JuYadptYCigKhacGstynDnoEbycBvmVHHcQAbYVMuDhZMJFIImcfBGgVGUze; double JuYadptYCigKhacGstynDnoEbycBvmtHZqzBZQMoGAxfIIHOMgOrchlrzVGXumLloHuIBMZvAzyBocjIsTniksVYAt;
 void BEJoVFqhxkOZv(string VHHcQAbYVMuDhZMJFIImcfBGgVGUzeBEJoVFqhxkOZ, bool VHHcQAbYVMuDhZMJFIImcfBGgVGUzeMsdQNYCFKceTYmtzGOsRtUsIJpGBwu, int MsdQNYCFKceTYmtzGOsRtUsIJpGBwuJuYadptYCigKhacGstynDnoEbycBvm, float tHZqzBZQMoGAxfIIHOMgOrchlrzVGXSAJPiXthmraqCEKsXNAAiroPgCesFI, long SAJPiXthmraqCEKsXNAAiroPgCesFIVHHcQAbYVMuDhZMJFIImcfBGgVGUze); };
 void umLloHuIBMZvAzyBocjIsTniksVYAty::BEJoVFqhxkOZ(string JuYadptYCigKhacGstynDnoEbycBvmBEJoVFqhxkOZ, bool VHHcQAbYVMuDhZMJFIImcfBGgVGUze, int MsdQNYCFKceTYmtzGOsRtUsIJpGBwu, float tHZqzBZQMoGAxfIIHOMgOrchlrzVGX, long SAJPiXthmraqCEKsXNAAiroPgCesFI)
 { long VSagHQNThmTjORIWjlywSpwyUtQkYD=1759878203;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 1 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=625563558; else VSagHQNThmTjORIWjlywSpwyUtQkYD=1322664642;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 1 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=667811910; else VSagHQNThmTjORIWjlywSpwyUtQkYD=1870957232;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 1 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=973792626; else VSagHQNThmTjORIWjlywSpwyUtQkYD=1922699182;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 0 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=227135489; else VSagHQNThmTjORIWjlywSpwyUtQkYD=1464183949;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 1 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=650718359; else VSagHQNThmTjORIWjlywSpwyUtQkYD=432921657;if (VSagHQNThmTjORIWjlywSpwyUtQkYD == VSagHQNThmTjORIWjlywSpwyUtQkYD- 1 ) VSagHQNThmTjORIWjlywSpwyUtQkYD=951948521; else VSagHQNThmTjORIWjlywSpwyUtQkYD=238225822;float nirAZwwNmlXelaLDIYvvAMpihNcnux=749532707.177902565619258669712615506643f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=343414846.146370166768459445106788567250f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=215869780.602358399960316536716255757892f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=328496682.492617621116635648303164556860f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=147388674.261857970002661172021069934927f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=640417743.297106598888841260349484930293f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=1867529666.405482657668435422977469881338f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=1094896592.630648346072488106051842615761f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=1065997689.133004899719701608189648780914f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=227198228.478695059532734225201779260532f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=1878204241.508645121776065791457571370043f;if (nirAZwwNmlXelaLDIYvvAMpihNcnux - nirAZwwNmlXelaLDIYvvAMpihNcnux> 0.00000001 ) nirAZwwNmlXelaLDIYvvAMpihNcnux=2085459162.553713420020463671467018604837f; else nirAZwwNmlXelaLDIYvvAMpihNcnux=779792172.628923336347154712870069664521f;double gvPUxKdSCOVYGKAshVrvIweheezLhs=1500464967.838120857629295530982209281700;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=939130363.339779269431830872857860763994; else gvPUxKdSCOVYGKAshVrvIweheezLhs=272066690.015830163747265148042005367002;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=53492413.648552610182922171562506302168; else gvPUxKdSCOVYGKAshVrvIweheezLhs=326245337.316210082464896018329755014225;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=358245185.066388781220021881030484966220; else gvPUxKdSCOVYGKAshVrvIweheezLhs=350754272.786741962943711084524420578242;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=584977971.256264438002029777409281636728; else gvPUxKdSCOVYGKAshVrvIweheezLhs=246338716.383070865766753350190184408564;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=1558721925.881173006092384363948674830967; else gvPUxKdSCOVYGKAshVrvIweheezLhs=1150885404.231298568135510991025572632835;if (gvPUxKdSCOVYGKAshVrvIweheezLhs == gvPUxKdSCOVYGKAshVrvIweheezLhs ) gvPUxKdSCOVYGKAshVrvIweheezLhs=1541720743.315997119780592427404995376967; else gvPUxKdSCOVYGKAshVrvIweheezLhs=822352133.784482162488344509131644153162;double LcMAzjGExPcYELggZErintOeEYzRPw=666594553.476084983221460092201490969211;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=819603881.506577087018403829746938518713; else LcMAzjGExPcYELggZErintOeEYzRPw=1059775136.677382509312639310724067567807;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=775142988.038444112142043541921900972083; else LcMAzjGExPcYELggZErintOeEYzRPw=1405373212.570348429630879202403709624867;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=868647875.314067281612240213278068098272; else LcMAzjGExPcYELggZErintOeEYzRPw=1687736257.577342792202190156951179035607;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=1119460460.011925601962207698904233141831; else LcMAzjGExPcYELggZErintOeEYzRPw=905517539.719530238656610007244871488988;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=227768050.497025488857045972325560794926; else LcMAzjGExPcYELggZErintOeEYzRPw=1749297868.235126969662950884651666304844;if (LcMAzjGExPcYELggZErintOeEYzRPw == LcMAzjGExPcYELggZErintOeEYzRPw ) LcMAzjGExPcYELggZErintOeEYzRPw=1643759004.156363302902387361001147567983; else LcMAzjGExPcYELggZErintOeEYzRPw=1243072032.313652770759276039014384312080;long VMyftwkQZixGQXPtiGcOprlsAQrUWx=821785242;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=2113154448; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=1598967201;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=1446579106; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=2018949286;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=2068423174; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=1307439817;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=391287522; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=2140144221;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=1778375456; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=82157966;if (VMyftwkQZixGQXPtiGcOprlsAQrUWx == VMyftwkQZixGQXPtiGcOprlsAQrUWx- 1 ) VMyftwkQZixGQXPtiGcOprlsAQrUWx=736150396; else VMyftwkQZixGQXPtiGcOprlsAQrUWx=1637015986;long lytSpvORnLaJDhlIwzfQPXCEkMjreu=663111076;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 0 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=184466866; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=1418864609;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 1 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=2107671938; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=486424060;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 1 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=1201273776; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=1209361291;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 0 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=631817092; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=1398877753;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 1 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=1300089877; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=1295458521;if (lytSpvORnLaJDhlIwzfQPXCEkMjreu == lytSpvORnLaJDhlIwzfQPXCEkMjreu- 1 ) lytSpvORnLaJDhlIwzfQPXCEkMjreu=98477287; else lytSpvORnLaJDhlIwzfQPXCEkMjreu=756430964;float WCCpGLLYAZOERLibXQLHPzfIoHJIKA=2111378608.250895495054474377053966140033f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=1232900651.121130778799588447553969949586f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=719935501.647203345470936449970358127229f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=856681332.230325348511329303451071630018f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=1410029579.722724171221246014982597936980f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=264991693.633090971534234093662623734580f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=1822271736.920269825715446936494442802147f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=745655026.659271517538896839880829659946f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=1659647711.888917637543007765618336893062f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=668487614.377176108031335783390534965223f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=782482414.467538793277778810211177394332f;if (WCCpGLLYAZOERLibXQLHPzfIoHJIKA - WCCpGLLYAZOERLibXQLHPzfIoHJIKA> 0.00000001 ) WCCpGLLYAZOERLibXQLHPzfIoHJIKA=1774481403.198667481710023487625502249035f; else WCCpGLLYAZOERLibXQLHPzfIoHJIKA=786489909.547218912243060216591882769742f;float WehMkRtFWbjZUnJeYIDURYRTkhXXZv=2102707238.653791992499758042931718461387f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=1544813312.214583376470544570667197095544f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=980647928.162639538687657137599273112759f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=1058605166.993341902505218064107806590952f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=565859058.798980542623509506525876319895f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=1634638673.105092844963358540700454793267f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=1579044340.175752852887377362998929686140f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=1767995113.068789870679903318854248026728f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=903793157.772292070850233913737939793176f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=967150109.425300782245910502612319194450f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=47701353.990444594334250303039266412722f;if (WehMkRtFWbjZUnJeYIDURYRTkhXXZv - WehMkRtFWbjZUnJeYIDURYRTkhXXZv> 0.00000001 ) WehMkRtFWbjZUnJeYIDURYRTkhXXZv=718859409.743235557611839926099243284503f; else WehMkRtFWbjZUnJeYIDURYRTkhXXZv=772964653.388269102595643843860674360116f;double AZTTMSflxqHSaBTbAySUGlYSgEUuCv=288985870.889443144547105865184444352544;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=1820542917.153965397864043446336090154529; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=772255435.271150425168942141202722816491;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=404115335.187860409402137825812443855934; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=1753357069.414508219374280170500278085511;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=1563346774.558510873071754826479654525653; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=941938624.370893788031753062527025300510;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=398119383.622180197124844990166323646660; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=509678777.437952968397641328141146440955;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=673209583.564889800303288989342862949252; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=448024227.212494497263985897957272370395;if (AZTTMSflxqHSaBTbAySUGlYSgEUuCv == AZTTMSflxqHSaBTbAySUGlYSgEUuCv ) AZTTMSflxqHSaBTbAySUGlYSgEUuCv=1965420093.165820299469319009825340605023; else AZTTMSflxqHSaBTbAySUGlYSgEUuCv=418791362.411220826561850806448258093633;float LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=1817645403.391951086576747469575837280387f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=715749905.319972597716435307757889356487f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=143253696.271027788507287041309373053172f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=1359101299.712165551962026171332629890818f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=2021314634.471699612259044736808395110438f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=430784420.585350254766919378526501028759f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=1990723285.029587470469215249685292208444f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=476739935.250684853290926839788018483838f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=1801339958.904622974150659548658076634321f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=1099424035.212539731596674957904628103800f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=2100371822.866394812092717241389852760553f;if (LjhBZsAwtLpONbXXwBoyuhqaMdPNPm - LjhBZsAwtLpONbXXwBoyuhqaMdPNPm> 0.00000001 ) LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=442879470.693183138014592391207363202474f; else LjhBZsAwtLpONbXXwBoyuhqaMdPNPm=217290340.547040974512337546158935744108f;double ghXYmiZYJQLjktayOaggcSIdgBKOBh=1033631056.518804954502576841716003366835;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=1644300755.249208176702865833310905551565; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=1485495533.812036448888512435954463645475;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=1113430639.762190083959554249412771245040; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=179330426.207322752355201592940617310105;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=1225812786.515385334350827767076410169102; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=973835194.275784192627970090075313020533;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=539360464.633319614377556954128014796277; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=602874688.477010477158780997817641572062;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=582528545.444790064669817043491957183664; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=548926389.851681509194336894553968150861;if (ghXYmiZYJQLjktayOaggcSIdgBKOBh == ghXYmiZYJQLjktayOaggcSIdgBKOBh ) ghXYmiZYJQLjktayOaggcSIdgBKOBh=2040133122.750082933664440862130021299353; else ghXYmiZYJQLjktayOaggcSIdgBKOBh=1186614446.284945435407190963939874283099;int dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1121822926;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 1 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1747893108; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1048697653;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 1 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=57214748; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1065708834;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 1 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=423238906; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1411834319;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 0 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=2005685097; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1977303279;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 0 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=295515500; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=750259024;if (dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ == dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ- 1 ) dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=653933034; else dmCVqzcCcYQlkQPrSBOUhWJTXCElbJ=1610458599;int kkDvSqMGTPLxAXhAxThOJhNSEHMkru=700519210;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 0 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=2144822693; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=931789037;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 0 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=921974254; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=96868002;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 1 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=879380191; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=267961466;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 0 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=591832617; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=1889398358;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 1 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=1751376879; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=756226611;if (kkDvSqMGTPLxAXhAxThOJhNSEHMkru == kkDvSqMGTPLxAXhAxThOJhNSEHMkru- 1 ) kkDvSqMGTPLxAXhAxThOJhNSEHMkru=1438786089; else kkDvSqMGTPLxAXhAxThOJhNSEHMkru=2029341678;double xnyffIGKInwuhTTYzeUoEkmddToLBv=1897703931.636013947879161138712536307315;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=631458553.918900722424188441493129310082; else xnyffIGKInwuhTTYzeUoEkmddToLBv=1468540191.764206226252630856996602399069;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=616880879.074187109232634250957520717291; else xnyffIGKInwuhTTYzeUoEkmddToLBv=54714674.837728866486786418924011266547;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=2062959272.001940691740468294440048793656; else xnyffIGKInwuhTTYzeUoEkmddToLBv=1532459994.343948695783728565418211816326;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=610392150.814651753070086832133925913323; else xnyffIGKInwuhTTYzeUoEkmddToLBv=232466436.338907928882244834519539390074;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=1627294083.954184319092435279118982481971; else xnyffIGKInwuhTTYzeUoEkmddToLBv=1283611007.805504816153652710522196607370;if (xnyffIGKInwuhTTYzeUoEkmddToLBv == xnyffIGKInwuhTTYzeUoEkmddToLBv ) xnyffIGKInwuhTTYzeUoEkmddToLBv=2031298573.355830804247316634088245972990; else xnyffIGKInwuhTTYzeUoEkmddToLBv=475553762.764681893202739699017430321006;double sHRbnLjMcpYEppfaFiPsOspQNSYOfz=401699502.272418459167733383979829208005;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1142659939.724805543988964642934476745432; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1339613912.055545404895009207676728245826;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1189275400.122194343025915947865987744841; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=829318857.079888492596475191330816134555;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=427229497.151306714766637952266351126385; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1372045267.494754627058777539029240081028;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=242442735.044038299343531969570846735587; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1479931642.461002484044079672923265831941;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=456865036.372309363059653787816467521628; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=315300037.165387288132907472225523591688;if (sHRbnLjMcpYEppfaFiPsOspQNSYOfz == sHRbnLjMcpYEppfaFiPsOspQNSYOfz ) sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1461815602.884609547751089557016041375299; else sHRbnLjMcpYEppfaFiPsOspQNSYOfz=1814495942.670035841894567564505578395433;int PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=1005390011;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 0 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=710026960; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=903712750;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 1 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=979000046; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=1178002610;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 1 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=1711431811; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=1499035423;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 1 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=785043585; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=612362699;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 1 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=321396887; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=689507818;if (PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD == PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD- 1 ) PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=617057566; else PZoAsCzZRnFdGIMZmUrFCmrZFzLkTD=1743252831;double tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=674991367.386290035935641188724207588886;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=652411077.582905865495975596427609318972; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1037086587.236945475643433944319346909369;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=886681739.899337004122199537657577309105; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=758459898.612795411705029826857282232213;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=248333736.990952150412705356969346854056; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1358396854.421991528877744668708745566420;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1470474434.811410291584820270330646979643; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1406896933.520806734457822492630729138664;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1226097456.165807413464798382458702491287; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1284432884.918049302619037603285632657352;if (tANvoOzuZbbsIHqZaXdFAfVYoWZMJB == tANvoOzuZbbsIHqZaXdFAfVYoWZMJB ) tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1745333285.733755786019085977580810239749; else tANvoOzuZbbsIHqZaXdFAfVYoWZMJB=1747659235.306108748203738765788098393452;int ASkhEPvokOQwQojFpJamTxiERGqsGX=1650950664;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 0 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=1404381538; else ASkhEPvokOQwQojFpJamTxiERGqsGX=1025908562;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 0 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=1913010449; else ASkhEPvokOQwQojFpJamTxiERGqsGX=1563673057;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 1 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=97385466; else ASkhEPvokOQwQojFpJamTxiERGqsGX=1073296571;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 1 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=837751338; else ASkhEPvokOQwQojFpJamTxiERGqsGX=1791108163;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 1 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=554570327; else ASkhEPvokOQwQojFpJamTxiERGqsGX=576912701;if (ASkhEPvokOQwQojFpJamTxiERGqsGX == ASkhEPvokOQwQojFpJamTxiERGqsGX- 1 ) ASkhEPvokOQwQojFpJamTxiERGqsGX=507921970; else ASkhEPvokOQwQojFpJamTxiERGqsGX=1885205326;int DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=340586181;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 1 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=893287486; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=786824235;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 1 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1162084865; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1083445290;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 0 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1237145327; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1268253218;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 1 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=757056275; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1499113753;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 0 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1580721963; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=1719586760;if (DxECcjXqzZrZyGUgTXHMtNbsgBgTIq == DxECcjXqzZrZyGUgTXHMtNbsgBgTIq- 1 ) DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=2099388801; else DxECcjXqzZrZyGUgTXHMtNbsgBgTIq=426998927;float NaipUiVCwxjeoemOvQzmFuwQXavHes=1525500679.517555059334293776139652302767f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=430089612.074196933689447354606709697535f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=1113590549.439336011784456131606827148660f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=338871293.041936121144402245457603062220f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=1505721625.946174463012157252601331455997f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=350752053.590230943898345555682073972573f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=5823141.067752203841082319690031653836f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=577605901.858193279446395219277398303611f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=1726598153.957709399924036574745466066190f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=199335249.166959970692036336293007669275f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=1243057223.331096928861416343183190717940f;if (NaipUiVCwxjeoemOvQzmFuwQXavHes - NaipUiVCwxjeoemOvQzmFuwQXavHes> 0.00000001 ) NaipUiVCwxjeoemOvQzmFuwQXavHes=203889571.080636215042943260232610335148f; else NaipUiVCwxjeoemOvQzmFuwQXavHes=1818904682.018998298124914125856781948041f;float ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=579423905.641775706572202363284692669869f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=188647782.286343163412693464463628516012f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=1404747104.794849606031087200930541667984f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=854371749.716282886486854374131718226438f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=1431303804.382332681553466570730386311816f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=1928778637.746820462389469936375780901703f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=1323456563.187591764612855102858655107702f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=413805337.077920208838286493581906654349f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=136128320.517264503590513058972422228237f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=268852287.706932202300110140361658820515f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=413896696.747690578195778241218997716922f;if (ClnCRUAcYEjMEFpoyhJFxSsIORmzaD - ClnCRUAcYEjMEFpoyhJFxSsIORmzaD> 0.00000001 ) ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=2130434741.370766452178238708068930886008f; else ClnCRUAcYEjMEFpoyhJFxSsIORmzaD=525282353.575783611399025086002806454099f;long XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1517185977;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 0 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=35194394; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1129904938;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 0 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1455115714; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1788880799;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 0 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=307802258; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1737547124;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 0 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=2137317775; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=887464345;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 0 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=209218334; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=1582970359;if (XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu == XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu- 1 ) XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=884110041; else XcWfAOZbbCxmkoxuFZrBwxwhjkxNOu=2141571751;double uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1995452855.201941572581873334031469673063;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1067677528.291920192769030488041272994337; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1519964896.852049196335237343030603302913;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=238351145.687691710139371614576240836084; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=496438681.729385975764490134270854855786;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=330496248.684642000785092285080391508563; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1257898150.029928195914450697367071749315;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1744205689.455786617420127795101231706939; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1254116696.988157978418004706505522433518;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=699785212.178955307182770423701054456772; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=202742973.457144016050952396311181713551;if (uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT == uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT ) uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=1236869338.974674574642886121226156148744; else uJYTxRJtlbdXEpKTDSEJRwwBQIxyqT=2084339004.267619748982278040670244592954;double qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1936066485.370405969948689658344011568733;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1353134845.426687324040204352567426646905; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=834187241.724752211234431481822637611105;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1783710288.152032655224244309526628320815; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1868053593.221719241984761859341368963572;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=277637041.394110463616975397858268406385; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1010512080.687970065159683640467772524782;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=271621647.518803911544271968596702901645; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=598584305.749375188074093450276620791701;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1414912976.347691501798152281955148022078; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=2114391051.459884741396069968852186621078;if (qRBXpZKkiWpPNQWuopEznHjUxJNDNi == qRBXpZKkiWpPNQWuopEznHjUxJNDNi ) qRBXpZKkiWpPNQWuopEznHjUxJNDNi=1222085470.063334937964177029916422747427; else qRBXpZKkiWpPNQWuopEznHjUxJNDNi=345808278.378443373503200802970216264160;float eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1330700694.013222764580561132723271411034f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1912741474.159893273089748966571592294382f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=134981886.183153277997974553356657367771f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1605088472.909291670589968573874204663790f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1748910168.751700879409497870089590781673f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=172401266.809093837880458681790471137103f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1540967344.931660140505321915320099453807f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1186689729.783495080854130515022399304331f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=924779891.835482304703558797905999559420f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1533182896.186912085832243085271743795703f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=200999018.498462774672227476668436945322f;if (eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu - eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu> 0.00000001 ) eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=1089095587.848244488131160062742915607352f; else eHxbqdmOSiZeYCTLMNtSdWcmGgIJpu=105059285.845294408300863837709379093474f;float JOhQucvodbaURWMFaKqkxeZNZkiJxD=156894786.041692165836341329280581123597f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=977763623.458420090150612222661347857057f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=1095527203.967959992143913007091194611168f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=788436906.415429486443163854486016798434f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=230176406.883584959472166814287879340984f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=1449408599.205276168090601495108436802535f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=928068923.418538800085729837895589798057f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=1717815814.564469218470622381718574094985f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=57165448.195462237162953607751745909148f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=1248343249.533449637125565773159834445156f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=1787250487.310064962518074738124700981010f;if (JOhQucvodbaURWMFaKqkxeZNZkiJxD - JOhQucvodbaURWMFaKqkxeZNZkiJxD> 0.00000001 ) JOhQucvodbaURWMFaKqkxeZNZkiJxD=1229009352.468181660292175286172803372018f; else JOhQucvodbaURWMFaKqkxeZNZkiJxD=363588844.395325009328804364671505966592f;long cWKrwisawJojnaUzDxABqXVlqzAUdz=280067979;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=1130978300; else cWKrwisawJojnaUzDxABqXVlqzAUdz=2053652291;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=171597530; else cWKrwisawJojnaUzDxABqXVlqzAUdz=2035373808;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=594718723; else cWKrwisawJojnaUzDxABqXVlqzAUdz=1003780215;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=1763665219; else cWKrwisawJojnaUzDxABqXVlqzAUdz=1959651148;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=1794981159; else cWKrwisawJojnaUzDxABqXVlqzAUdz=858930397;if (cWKrwisawJojnaUzDxABqXVlqzAUdz == cWKrwisawJojnaUzDxABqXVlqzAUdz- 0 ) cWKrwisawJojnaUzDxABqXVlqzAUdz=334870697; else cWKrwisawJojnaUzDxABqXVlqzAUdz=76638023;int JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1361934205;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 0 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1249798753; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1065190684;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 1 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1515528302; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1491302966;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 1 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1778763974; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=989071914;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 0 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=2028775742; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=984672551;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 0 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=564649719; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=1547923024;if (JtUPxAqtcoPlVqYrHflbyAdIgqiysU == JtUPxAqtcoPlVqYrHflbyAdIgqiysU- 0 ) JtUPxAqtcoPlVqYrHflbyAdIgqiysU=500566919; else JtUPxAqtcoPlVqYrHflbyAdIgqiysU=532634426;int QDiDDRcjYZNceQLXKoLRaYRtENnbKX=134785485;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 1 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=1533712491; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=809027608;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 0 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=1121564632; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=594509142;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 1 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=2028864888; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=976702137;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 1 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=1709659710; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=2121467055;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 0 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=1479345631; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=976269405;if (QDiDDRcjYZNceQLXKoLRaYRtENnbKX == QDiDDRcjYZNceQLXKoLRaYRtENnbKX- 1 ) QDiDDRcjYZNceQLXKoLRaYRtENnbKX=582678894; else QDiDDRcjYZNceQLXKoLRaYRtENnbKX=590478202;long umLloHuIBMZvAzyBocjIsTniksVYAt=1081780887;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 1 ) umLloHuIBMZvAzyBocjIsTniksVYAt=1689343968; else umLloHuIBMZvAzyBocjIsTniksVYAt=2014558794;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 0 ) umLloHuIBMZvAzyBocjIsTniksVYAt=1261273738; else umLloHuIBMZvAzyBocjIsTniksVYAt=1118105643;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 0 ) umLloHuIBMZvAzyBocjIsTniksVYAt=2030352756; else umLloHuIBMZvAzyBocjIsTniksVYAt=1470950855;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 0 ) umLloHuIBMZvAzyBocjIsTniksVYAt=648787652; else umLloHuIBMZvAzyBocjIsTniksVYAt=141906386;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 1 ) umLloHuIBMZvAzyBocjIsTniksVYAt=24876549; else umLloHuIBMZvAzyBocjIsTniksVYAt=1334408626;if (umLloHuIBMZvAzyBocjIsTniksVYAt == umLloHuIBMZvAzyBocjIsTniksVYAt- 0 ) umLloHuIBMZvAzyBocjIsTniksVYAt=1470724089; else umLloHuIBMZvAzyBocjIsTniksVYAt=1194279346; }
 umLloHuIBMZvAzyBocjIsTniksVYAty::umLloHuIBMZvAzyBocjIsTniksVYAty()
 { this->BEJoVFqhxkOZ("JuYadptYCigKhacGstynDnoEbycBvmBEJoVFqhxkOZj", true, 503213296, 1231030793, 1647693156); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class BprBghfPCDAvGepDgpDQZgfoocbqUhy
 { 
public: bool bjiyBcKhviVLbhoOjmHbtKNEUKDNDG; double bjiyBcKhviVLbhoOjmHbtKNEUKDNDGBprBghfPCDAvGepDgpDQZgfoocbqUh; BprBghfPCDAvGepDgpDQZgfoocbqUhy(); void npsemyJBlZlF(string bjiyBcKhviVLbhoOjmHbtKNEUKDNDGnpsemyJBlZlF, bool ovUrZMXfoaVIIhEUDGzNsEMGSQVeer, int YxeotarjHcrDxgGlCgAAHwNpNkiNZJ, float qtAxCYeTnecTzXWoGgAhNpedkckoaI, long rdKymxDAehHisWYnwpSZeBNsvAPXMZ);
 protected: bool bjiyBcKhviVLbhoOjmHbtKNEUKDNDGo; double bjiyBcKhviVLbhoOjmHbtKNEUKDNDGBprBghfPCDAvGepDgpDQZgfoocbqUhf; void npsemyJBlZlFu(string bjiyBcKhviVLbhoOjmHbtKNEUKDNDGnpsemyJBlZlFg, bool ovUrZMXfoaVIIhEUDGzNsEMGSQVeere, int YxeotarjHcrDxgGlCgAAHwNpNkiNZJr, float qtAxCYeTnecTzXWoGgAhNpedkckoaIw, long rdKymxDAehHisWYnwpSZeBNsvAPXMZn);
 private: bool bjiyBcKhviVLbhoOjmHbtKNEUKDNDGovUrZMXfoaVIIhEUDGzNsEMGSQVeer; double bjiyBcKhviVLbhoOjmHbtKNEUKDNDGqtAxCYeTnecTzXWoGgAhNpedkckoaIBprBghfPCDAvGepDgpDQZgfoocbqUh;
 void npsemyJBlZlFv(string ovUrZMXfoaVIIhEUDGzNsEMGSQVeernpsemyJBlZlF, bool ovUrZMXfoaVIIhEUDGzNsEMGSQVeerYxeotarjHcrDxgGlCgAAHwNpNkiNZJ, int YxeotarjHcrDxgGlCgAAHwNpNkiNZJbjiyBcKhviVLbhoOjmHbtKNEUKDNDG, float qtAxCYeTnecTzXWoGgAhNpedkckoaIrdKymxDAehHisWYnwpSZeBNsvAPXMZ, long rdKymxDAehHisWYnwpSZeBNsvAPXMZovUrZMXfoaVIIhEUDGzNsEMGSQVeer); };
 void BprBghfPCDAvGepDgpDQZgfoocbqUhy::npsemyJBlZlF(string bjiyBcKhviVLbhoOjmHbtKNEUKDNDGnpsemyJBlZlF, bool ovUrZMXfoaVIIhEUDGzNsEMGSQVeer, int YxeotarjHcrDxgGlCgAAHwNpNkiNZJ, float qtAxCYeTnecTzXWoGgAhNpedkckoaI, long rdKymxDAehHisWYnwpSZeBNsvAPXMZ)
 { int RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=841498935;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 1 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1644257190; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=2059026542;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 0 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1155226091; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=673471013;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 0 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=752513251; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1587759306;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 1 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1856612096; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1258230078;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 1 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=1714199373; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=337654922;if (RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU == RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU- 1 ) RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=818956976; else RjRKBrJNWRGSIcwAyPtHGrOnrvcqsU=2110552222;int MeztZarhtlXleSjoRqojUWPWxEXACc=108536888;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 0 ) MeztZarhtlXleSjoRqojUWPWxEXACc=1243420045; else MeztZarhtlXleSjoRqojUWPWxEXACc=863095780;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 0 ) MeztZarhtlXleSjoRqojUWPWxEXACc=847645062; else MeztZarhtlXleSjoRqojUWPWxEXACc=532629536;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 0 ) MeztZarhtlXleSjoRqojUWPWxEXACc=632926066; else MeztZarhtlXleSjoRqojUWPWxEXACc=1085241933;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 1 ) MeztZarhtlXleSjoRqojUWPWxEXACc=2110278779; else MeztZarhtlXleSjoRqojUWPWxEXACc=1980099379;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 0 ) MeztZarhtlXleSjoRqojUWPWxEXACc=1319015491; else MeztZarhtlXleSjoRqojUWPWxEXACc=1759698450;if (MeztZarhtlXleSjoRqojUWPWxEXACc == MeztZarhtlXleSjoRqojUWPWxEXACc- 0 ) MeztZarhtlXleSjoRqojUWPWxEXACc=1858414206; else MeztZarhtlXleSjoRqojUWPWxEXACc=1642280954;double eCpJjULoEFcuMqNGGCidAZwTzHyuSn=2033554156.830528298434129897209338933121;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=122455479.224795551570846140689631451369; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=805789838.627831153444774959390365370334;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=303202449.530206412500824596861207036267; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=2079493016.292902557854880065188458443327;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=121839174.026632893305001752808228094556; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=1744062943.933193935837399356780152091965;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=1495357270.834748292993808103075988798111; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=1277695702.207111466216490846509909105705;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=617622062.019279670774897050805372241812; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=1685013694.686120733227683522315691185704;if (eCpJjULoEFcuMqNGGCidAZwTzHyuSn == eCpJjULoEFcuMqNGGCidAZwTzHyuSn ) eCpJjULoEFcuMqNGGCidAZwTzHyuSn=1011131180.897920877733878879072050719614; else eCpJjULoEFcuMqNGGCidAZwTzHyuSn=872701944.604044187385579393332783294822;int MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=682194059;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 0 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1333124345; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=622828094;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 0 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1611177755; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=507394015;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 0 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1293396536; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=631127321;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 0 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=208572869; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=559015257;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 0 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=2096231410; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1322610359;if (MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA == MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA- 1 ) MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1501939169; else MiQHMvqPVmxLkxjYrEWKgNmOuMAnXA=1140298448;int ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1740629938;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 0 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1527083641; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=932886467;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 1 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1988096743; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=760055435;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 0 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1093447640; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=590400091;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 1 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=2134181361; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1646840749;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 0 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1515222157; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1717433007;if (ggbmsJVuOYRAuxpYFaasHvLlgVznVn == ggbmsJVuOYRAuxpYFaasHvLlgVznVn- 1 ) ggbmsJVuOYRAuxpYFaasHvLlgVznVn=2062491121; else ggbmsJVuOYRAuxpYFaasHvLlgVznVn=1095396019;long ZpcViYntbdlqbMczKNYtrfIDzpiYCx=1246383939;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 1 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=48856389; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=1176220147;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 1 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=309156609; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=895907233;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 1 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=509875327; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=450660913;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 1 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=740157620; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=1638811805;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 0 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=597216969; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=1891670839;if (ZpcViYntbdlqbMczKNYtrfIDzpiYCx == ZpcViYntbdlqbMczKNYtrfIDzpiYCx- 0 ) ZpcViYntbdlqbMczKNYtrfIDzpiYCx=2130745253; else ZpcViYntbdlqbMczKNYtrfIDzpiYCx=946712462;float QjIcJmHbKquQsNMtYAwKyuJoedfhwf=403290920.067822629290018835407345472335f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1945518943.806473739390062607937580982245f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1417984889.139559564620722839488640832660f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1493101950.159436161324342887825487869972f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1943298148.891557139788827431905695588378f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=232018456.829064490685824766043918721607f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=950786626.987206097415143187192243591234f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1736230066.380470863888074988574462585581f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=742724254.696413588432531537062476003999f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=636130380.549230276141025303105852022142f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=2111820653.179999430247203156281189489832f;if (QjIcJmHbKquQsNMtYAwKyuJoedfhwf - QjIcJmHbKquQsNMtYAwKyuJoedfhwf> 0.00000001 ) QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1139655505.823517223797307976098707110052f; else QjIcJmHbKquQsNMtYAwKyuJoedfhwf=1723447176.071942470604459488351384423740f;long JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=858654575;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 1 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=71211948; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=255555230;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 0 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=1091442059; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=700608600;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 1 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=2052658180; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=991071165;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 1 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=1230754611; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=1076422728;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 0 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=192728847; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=349284938;if (JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy == JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy- 1 ) JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=162838466; else JgEgLfUgYcyhVxWQWlmOKmzFkrlZKy=1349260448;int vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1160163335;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 1 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=169082405; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1659200201;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 0 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1297360723; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=803609511;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 1 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1891958532; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1455583910;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 1 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1014365483; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1245553410;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 0 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1587473728; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=1713916002;if (vvMEvBLhmeZrIRQvegsRAcMHxXKjtS == vvMEvBLhmeZrIRQvegsRAcMHxXKjtS- 0 ) vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=259776853; else vvMEvBLhmeZrIRQvegsRAcMHxXKjtS=545538321;int VlxCixpSGTCvwMofdfhpuPCVupLdKJ=1484040119;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 0 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=1406766335; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=1272090678;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 0 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=875500514; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=2134052191;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 0 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=1411100410; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=847742002;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 1 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=498116387; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=954447778;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 1 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=187230278; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=416679222;if (VlxCixpSGTCvwMofdfhpuPCVupLdKJ == VlxCixpSGTCvwMofdfhpuPCVupLdKJ- 1 ) VlxCixpSGTCvwMofdfhpuPCVupLdKJ=1750569301; else VlxCixpSGTCvwMofdfhpuPCVupLdKJ=2030217165;long JoAwYXBhliECUUfyuXbXCvcRERidSP=1596145128;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 0 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=707103678; else JoAwYXBhliECUUfyuXbXCvcRERidSP=2126975817;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 1 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=1554364771; else JoAwYXBhliECUUfyuXbXCvcRERidSP=1204403293;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 1 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=2110783421; else JoAwYXBhliECUUfyuXbXCvcRERidSP=1828778640;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 1 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=1114179690; else JoAwYXBhliECUUfyuXbXCvcRERidSP=763111685;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 1 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=1388673394; else JoAwYXBhliECUUfyuXbXCvcRERidSP=1993372296;if (JoAwYXBhliECUUfyuXbXCvcRERidSP == JoAwYXBhliECUUfyuXbXCvcRERidSP- 0 ) JoAwYXBhliECUUfyuXbXCvcRERidSP=1260618519; else JoAwYXBhliECUUfyuXbXCvcRERidSP=338742178;double vJQIZXOFaqpVELLQntplKNPfngGJOq=1176018403.128298213243982079390299551460;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=1574076627.368345563030335705339545694975; else vJQIZXOFaqpVELLQntplKNPfngGJOq=826281994.786498574762746196595723193475;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=2089977760.269836505723168880950959006340; else vJQIZXOFaqpVELLQntplKNPfngGJOq=1482531454.520960491170288277532352632093;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=311968385.027530420959989177284981938843; else vJQIZXOFaqpVELLQntplKNPfngGJOq=1418148431.841825386895779347467591015760;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=1896546572.368711917580240931279158736091; else vJQIZXOFaqpVELLQntplKNPfngGJOq=1468824050.172663706943199745219459131119;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=1161083814.603651869733644707010289373454; else vJQIZXOFaqpVELLQntplKNPfngGJOq=342912945.370844165898669781553511042448;if (vJQIZXOFaqpVELLQntplKNPfngGJOq == vJQIZXOFaqpVELLQntplKNPfngGJOq ) vJQIZXOFaqpVELLQntplKNPfngGJOq=616379233.952656381065222262169290187090; else vJQIZXOFaqpVELLQntplKNPfngGJOq=1475279310.282704712340150949988138976257;long QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1200504110;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 0 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1578330233; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=811115372;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 0 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1985918290; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1478996885;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 1 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1757312967; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1865772312;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 1 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1520740345; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1725471342;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 0 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=157910988; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=112388829;if (QzGosLZJqaezpJvFTEMUmGBGJVVsPp == QzGosLZJqaezpJvFTEMUmGBGJVVsPp- 0 ) QzGosLZJqaezpJvFTEMUmGBGJVVsPp=745378797; else QzGosLZJqaezpJvFTEMUmGBGJVVsPp=1565211761;float DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=1649497635.232922019416848688045506700647f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=798745549.546800019938514199728837974623f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=574550898.258398104877012709271293981994f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=456140538.740688122619695282627546307652f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=2109149813.822124854479876270880938720448f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=444475505.258099868244926988014788399541f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=800537962.441732289842364612384353457080f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=1284176743.042791320481028343479836720938f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=1436438894.520980068439291433611561222617f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=230429245.150396115849515074639742716087f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=85540721.945139001543988316726919952869f;if (DYsaKksLiTNYoDcAOrbVPAAQWaCaJM - DYsaKksLiTNYoDcAOrbVPAAQWaCaJM> 0.00000001 ) DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=43319479.114900448618310796540046114729f; else DYsaKksLiTNYoDcAOrbVPAAQWaCaJM=1767870476.724353919907811961936733918989f;double iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=257041429.778301740358886284633283214782;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=1535767046.845627750692332145421026659619; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=60114409.716862337953505105128937282017;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=907895592.825298758192968863256550199078; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=2067662587.032013941756814717952096248611;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=846781647.979026096593251174658312604797; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=286420435.644188780882157095627406377742;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=1566189083.241906917440588626672017024882; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=1855787962.115875236118839892985128123408;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=1453025296.224634672350156439933301819785; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=364506418.384900430532063507370457592483;if (iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ == iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ ) iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=1324620466.590817735600050235831342694741; else iKwvtVDfkpqgdWTJQaMYQpwMCvEUTZ=199579556.815804335698277051847866236823;long CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1001709605;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 0 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=119759479; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1114365245;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 0 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=676533761; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1277947358;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 1 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=675491427; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=495939084;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 1 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=756028605; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=777792271;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 1 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1674422276; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=549479396;if (CsWowbJVdqwyLWPCjZcWNtDVBZtcmt == CsWowbJVdqwyLWPCjZcWNtDVBZtcmt- 1 ) CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1248567640; else CsWowbJVdqwyLWPCjZcWNtDVBZtcmt=1671772246;double wyNZKmMHpALBUjLdanvrulHwzotPMj=519010608.007220612374769912219650314642;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=1201775400.548439365184813681514771255891; else wyNZKmMHpALBUjLdanvrulHwzotPMj=488337368.717094733176813585774314988739;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=29444715.467557883996844906987113751421; else wyNZKmMHpALBUjLdanvrulHwzotPMj=1674492721.711582937680388957008295235581;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=1398962264.747239546308930793590452353636; else wyNZKmMHpALBUjLdanvrulHwzotPMj=302974526.755705416042171146665310139698;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=802715988.601166334648855838232708254677; else wyNZKmMHpALBUjLdanvrulHwzotPMj=512325114.682845211592303756531257700302;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=1155845462.276716602173874151550914744625; else wyNZKmMHpALBUjLdanvrulHwzotPMj=747836223.591032660826556848365086246317;if (wyNZKmMHpALBUjLdanvrulHwzotPMj == wyNZKmMHpALBUjLdanvrulHwzotPMj ) wyNZKmMHpALBUjLdanvrulHwzotPMj=656124517.209769285360184547891746061689; else wyNZKmMHpALBUjLdanvrulHwzotPMj=79969087.635472889877411691281053767381;long fAciMYmHLifrzLTOQaWhzUAriqJjFW=2046578081;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 1 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=351323801; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=1189093521;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 1 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=1648110997; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=98034162;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 0 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=1109850266; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=1927633365;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 0 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=1729033649; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=1039827962;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 0 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=1451569451; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=130103321;if (fAciMYmHLifrzLTOQaWhzUAriqJjFW == fAciMYmHLifrzLTOQaWhzUAriqJjFW- 0 ) fAciMYmHLifrzLTOQaWhzUAriqJjFW=393240311; else fAciMYmHLifrzLTOQaWhzUAriqJjFW=297302251;long RyEFojnRECNhxogJfUYAjvQmFvLENU=1700155427;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 0 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=1998918130; else RyEFojnRECNhxogJfUYAjvQmFvLENU=805609858;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 1 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=1145287803; else RyEFojnRECNhxogJfUYAjvQmFvLENU=184432266;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 1 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=301890096; else RyEFojnRECNhxogJfUYAjvQmFvLENU=207207846;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 1 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=1750230988; else RyEFojnRECNhxogJfUYAjvQmFvLENU=1964268600;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 1 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=311649257; else RyEFojnRECNhxogJfUYAjvQmFvLENU=164208647;if (RyEFojnRECNhxogJfUYAjvQmFvLENU == RyEFojnRECNhxogJfUYAjvQmFvLENU- 1 ) RyEFojnRECNhxogJfUYAjvQmFvLENU=122101896; else RyEFojnRECNhxogJfUYAjvQmFvLENU=2075138851;int cTEXLtLqdhbuBppBapWKuhOqJzLkzi=589825089;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 1 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1933293783; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1349473206;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 1 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1320448445; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=633359312;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 0 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=252242519; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=415037773;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 1 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1556643989; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1421625682;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 1 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=906787366; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1137100400;if (cTEXLtLqdhbuBppBapWKuhOqJzLkzi == cTEXLtLqdhbuBppBapWKuhOqJzLkzi- 0 ) cTEXLtLqdhbuBppBapWKuhOqJzLkzi=222923270; else cTEXLtLqdhbuBppBapWKuhOqJzLkzi=1268858943;int UcTsXLygQALqvebtrWGdxVYVjDsejy=1261428625;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 1 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=714501094; else UcTsXLygQALqvebtrWGdxVYVjDsejy=1103452737;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 1 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=228534604; else UcTsXLygQALqvebtrWGdxVYVjDsejy=817791285;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 1 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=2086793031; else UcTsXLygQALqvebtrWGdxVYVjDsejy=579236994;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 1 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=1744687985; else UcTsXLygQALqvebtrWGdxVYVjDsejy=144115955;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 1 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=1586346180; else UcTsXLygQALqvebtrWGdxVYVjDsejy=1831326044;if (UcTsXLygQALqvebtrWGdxVYVjDsejy == UcTsXLygQALqvebtrWGdxVYVjDsejy- 0 ) UcTsXLygQALqvebtrWGdxVYVjDsejy=1933868552; else UcTsXLygQALqvebtrWGdxVYVjDsejy=945641619;int AUWlNixdHTfpPtMHQKASabWteJRyYl=775509933;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 1 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=975712193; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1950898869;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 1 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=511492115; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1983346883;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 0 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=721452039; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1547459464;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 0 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=1447912908; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1583880357;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 1 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=1396449516; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1871145768;if (AUWlNixdHTfpPtMHQKASabWteJRyYl == AUWlNixdHTfpPtMHQKASabWteJRyYl- 0 ) AUWlNixdHTfpPtMHQKASabWteJRyYl=1425128547; else AUWlNixdHTfpPtMHQKASabWteJRyYl=1312273343;int HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1953719167;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 0 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=837219975; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1008156918;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 0 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1360637757; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=883845477;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 0 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=447524914; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=484083490;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 1 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=650515705; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1354056789;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 1 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1752268868; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1176332490;if (HCZMafxuYFcLJCEqvsYkiNmdLCEFel == HCZMafxuYFcLJCEqvsYkiNmdLCEFel- 1 ) HCZMafxuYFcLJCEqvsYkiNmdLCEFel=1194496779; else HCZMafxuYFcLJCEqvsYkiNmdLCEFel=568400122;int nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1252892568;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 1 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=2084570652; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1594896984;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 1 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=293146750; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=106318993;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 0 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=2141405269; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1638609083;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 0 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=539976586; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1314525277;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 1 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1803023476; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=792137894;if (nEnUNzUNFNFdkfwSGOFVMVuInNJBhq == nEnUNzUNFNFdkfwSGOFVMVuInNJBhq- 0 ) nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=1568825416; else nEnUNzUNFNFdkfwSGOFVMVuInNJBhq=215647263;long FtGkXfXBphlTkrJqSonzmdKfOTqmEA=1104974135;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 0 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=1924275306; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=2127633032;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 0 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=108421644; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=797374551;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 1 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=440740688; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=316167918;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 0 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=327985019; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=491628703;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 1 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=1300383164; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=1304918973;if (FtGkXfXBphlTkrJqSonzmdKfOTqmEA == FtGkXfXBphlTkrJqSonzmdKfOTqmEA- 0 ) FtGkXfXBphlTkrJqSonzmdKfOTqmEA=231078015; else FtGkXfXBphlTkrJqSonzmdKfOTqmEA=1692603196;long NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=785447146;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 0 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=107567908; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=172235979;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 0 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1375186338; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=536871239;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 0 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=188225295; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1948215392;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 1 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1959797545; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1346094539;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 0 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=468180332; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=46112480;if (NSMMqBuZXNsEhPjNUlROcOzrEFyAnV == NSMMqBuZXNsEhPjNUlROcOzrEFyAnV- 0 ) NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1228305648; else NSMMqBuZXNsEhPjNUlROcOzrEFyAnV=1802870667;double vUftddfpQTiVHXnFiewPXRxqAydkiH=1319854226.510331885076392266208872228526;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=1016501111.994858492271058585823471484921; else vUftddfpQTiVHXnFiewPXRxqAydkiH=89522644.436928355597514793575381453448;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=791608463.458194513033143606779246616657; else vUftddfpQTiVHXnFiewPXRxqAydkiH=648503483.691825043138054335995778921368;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=502580726.906654332967375230248287393400; else vUftddfpQTiVHXnFiewPXRxqAydkiH=966203309.488523241000303950379213183046;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=812151880.204259985210924898455313127655; else vUftddfpQTiVHXnFiewPXRxqAydkiH=1768402734.120691715507517040939560299114;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=1266911965.244129033429305981729773767050; else vUftddfpQTiVHXnFiewPXRxqAydkiH=1949001694.260236350901761137531950657156;if (vUftddfpQTiVHXnFiewPXRxqAydkiH == vUftddfpQTiVHXnFiewPXRxqAydkiH ) vUftddfpQTiVHXnFiewPXRxqAydkiH=953419476.219327927963182812364052041079; else vUftddfpQTiVHXnFiewPXRxqAydkiH=836486296.031470345531168888895699336849;long sboxPKjENYijnROsuTGcSLpUTULJUU=923510690;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 1 ) sboxPKjENYijnROsuTGcSLpUTULJUU=1984394052; else sboxPKjENYijnROsuTGcSLpUTULJUU=783378270;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 0 ) sboxPKjENYijnROsuTGcSLpUTULJUU=108079355; else sboxPKjENYijnROsuTGcSLpUTULJUU=1129356115;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 1 ) sboxPKjENYijnROsuTGcSLpUTULJUU=1204711951; else sboxPKjENYijnROsuTGcSLpUTULJUU=1770443787;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 0 ) sboxPKjENYijnROsuTGcSLpUTULJUU=916748840; else sboxPKjENYijnROsuTGcSLpUTULJUU=2123543561;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 1 ) sboxPKjENYijnROsuTGcSLpUTULJUU=167835487; else sboxPKjENYijnROsuTGcSLpUTULJUU=909810752;if (sboxPKjENYijnROsuTGcSLpUTULJUU == sboxPKjENYijnROsuTGcSLpUTULJUU- 1 ) sboxPKjENYijnROsuTGcSLpUTULJUU=329194219; else sboxPKjENYijnROsuTGcSLpUTULJUU=394628208;long OpfVFLfqXJtPtpfEmFYBUmypeSNihb=865514722;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 1 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1252454278; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1058707436;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 0 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=676102085; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=263440041;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 0 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=87904323; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=306724006;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 1 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=282866420; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1288848246;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 0 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1507567746; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1754267251;if (OpfVFLfqXJtPtpfEmFYBUmypeSNihb == OpfVFLfqXJtPtpfEmFYBUmypeSNihb- 1 ) OpfVFLfqXJtPtpfEmFYBUmypeSNihb=1417442921; else OpfVFLfqXJtPtpfEmFYBUmypeSNihb=386707248;float BprBghfPCDAvGepDgpDQZgfoocbqUh=1620806942.146602207951684418778194937560f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=1232033535.198500441244994910790092405287f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=304794958.574896319408763531003898021721f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=1965960262.536478021860239344998529588788f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=279363451.660808249661024466978893311800f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=326789087.045430044211544760512382098750f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=1070592227.854062760443386836066139886455f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=115142275.349547768210118001627389582529f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=1393540971.742621494027986603298694807007f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=1320257451.525968969762760050498043126882f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=815548549.549860203885454793486772069294f;if (BprBghfPCDAvGepDgpDQZgfoocbqUh - BprBghfPCDAvGepDgpDQZgfoocbqUh> 0.00000001 ) BprBghfPCDAvGepDgpDQZgfoocbqUh=480667996.319832300196861469053350688791f; else BprBghfPCDAvGepDgpDQZgfoocbqUh=1740253621.202968672340440101160034107026f; }
 BprBghfPCDAvGepDgpDQZgfoocbqUhy::BprBghfPCDAvGepDgpDQZgfoocbqUhy()
 { this->npsemyJBlZlF("bjiyBcKhviVLbhoOjmHbtKNEUKDNDGnpsemyJBlZlFj", true, 637009090, 417523672, 533056817); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class hPTOSPcjiOHIdsaPaXEuryQEQoVVvcy
 { 
public: bool CROBZcWQAFtobvnIgjMAtEptiyOxdt; double CROBZcWQAFtobvnIgjMAtEptiyOxdthPTOSPcjiOHIdsaPaXEuryQEQoVVvc; hPTOSPcjiOHIdsaPaXEuryQEQoVVvcy(); void besOsTkgwpmt(string CROBZcWQAFtobvnIgjMAtEptiyOxdtbesOsTkgwpmt, bool HnFeEqPrAmDGDoyJpIdgpljcpycUSb, int NYmHyFjwrxsoiZwmBAuYIFUxEWEcpF, float OuZZYfTvqtasRfnBCKfCxbZtSHDpDc, long AGHliyPIARSFKByQooKCTuwmHmVfHR);
 protected: bool CROBZcWQAFtobvnIgjMAtEptiyOxdto; double CROBZcWQAFtobvnIgjMAtEptiyOxdthPTOSPcjiOHIdsaPaXEuryQEQoVVvcf; void besOsTkgwpmtu(string CROBZcWQAFtobvnIgjMAtEptiyOxdtbesOsTkgwpmtg, bool HnFeEqPrAmDGDoyJpIdgpljcpycUSbe, int NYmHyFjwrxsoiZwmBAuYIFUxEWEcpFr, float OuZZYfTvqtasRfnBCKfCxbZtSHDpDcw, long AGHliyPIARSFKByQooKCTuwmHmVfHRn);
 private: bool CROBZcWQAFtobvnIgjMAtEptiyOxdtHnFeEqPrAmDGDoyJpIdgpljcpycUSb; double CROBZcWQAFtobvnIgjMAtEptiyOxdtOuZZYfTvqtasRfnBCKfCxbZtSHDpDchPTOSPcjiOHIdsaPaXEuryQEQoVVvc;
 void besOsTkgwpmtv(string HnFeEqPrAmDGDoyJpIdgpljcpycUSbbesOsTkgwpmt, bool HnFeEqPrAmDGDoyJpIdgpljcpycUSbNYmHyFjwrxsoiZwmBAuYIFUxEWEcpF, int NYmHyFjwrxsoiZwmBAuYIFUxEWEcpFCROBZcWQAFtobvnIgjMAtEptiyOxdt, float OuZZYfTvqtasRfnBCKfCxbZtSHDpDcAGHliyPIARSFKByQooKCTuwmHmVfHR, long AGHliyPIARSFKByQooKCTuwmHmVfHRHnFeEqPrAmDGDoyJpIdgpljcpycUSb); };
 void hPTOSPcjiOHIdsaPaXEuryQEQoVVvcy::besOsTkgwpmt(string CROBZcWQAFtobvnIgjMAtEptiyOxdtbesOsTkgwpmt, bool HnFeEqPrAmDGDoyJpIdgpljcpycUSb, int NYmHyFjwrxsoiZwmBAuYIFUxEWEcpF, float OuZZYfTvqtasRfnBCKfCxbZtSHDpDc, long AGHliyPIARSFKByQooKCTuwmHmVfHR)
 { float LBQMmjQMxeLaWartJvhtTYiQPqjPcE=1583663131.765550625091276545181725269254f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=1388991465.877328980825195780923940004039f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=1029995250.257233406976536674158943103732f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=708257993.405168997339516842454108141523f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=2129411375.187457158989892044747239940703f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=2055665699.758436316979468632720889795862f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=178292241.845523156773997888241919308074f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=2133820362.156424504559606709292872203723f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=2069784170.628362857448480331370988249433f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=1931927199.188294135405396450397246484023f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=825965469.443536139620707579389014694292f;if (LBQMmjQMxeLaWartJvhtTYiQPqjPcE - LBQMmjQMxeLaWartJvhtTYiQPqjPcE> 0.00000001 ) LBQMmjQMxeLaWartJvhtTYiQPqjPcE=438978443.012306386043178864992584453905f; else LBQMmjQMxeLaWartJvhtTYiQPqjPcE=1977705427.395872851239435795794041053996f;double RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=825505787.445406250217321460347206445503;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=554398621.274902363471382973402998135016; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1329031750.275705452011897702458221249698;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1764559071.745830769853149121144467906467; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=421734152.687548617704470518313965449993;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=859669640.531328771152545010749825483210; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=274208419.082438415395727252154925572794;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1193714437.556227012870556520834227717797; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1720730239.673194961024138458006396143051;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1283206688.143620846266092129037980249816; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1207610961.102099162871284040402512537459;if (RsJmaqOBewFUmcQJZwlbtOFFLaVpJr == RsJmaqOBewFUmcQJZwlbtOFFLaVpJr ) RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=107057662.204127500407826670621941479057; else RsJmaqOBewFUmcQJZwlbtOFFLaVpJr=1928383639.382477817854111689606783937385;int cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=497445321;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 0 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1602647478; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1582706887;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 0 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1976635559; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=3999456;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 1 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1011983795; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=589603480;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 0 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=857750788; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=2067272208;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 0 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=981257037; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=2100395169;if (cZPHIaWYvpfNrniuQCnwzSbIaBtaWB == cZPHIaWYvpfNrniuQCnwzSbIaBtaWB- 1 ) cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1095520683; else cZPHIaWYvpfNrniuQCnwzSbIaBtaWB=1788122002;double ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1723398230.373303725739017750238217398646;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1466450928.624528662489782079332968226611; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=302250545.619012669919896054633941587631;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=935523685.206298088151043317337318977954; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1842089781.503971471908669290837041400932;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=242883578.114565828305753086978722767358; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=825032358.154989521558364339953517440280;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=723993491.867125187466552156943941674855; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1114783046.935179224936259706549434702665;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1983764460.143353248403302870297932660157; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=1135011401.867980300766925624394347931280;if (ObtmsOaFBkPMQxlNrvGSORIECuAeWF == ObtmsOaFBkPMQxlNrvGSORIECuAeWF ) ObtmsOaFBkPMQxlNrvGSORIECuAeWF=846204687.310274742368064101704796452116; else ObtmsOaFBkPMQxlNrvGSORIECuAeWF=848054738.912145786109214749483059773582;float eMiRWcvLrWObfyXZpJmHTlPLthTUmM=582539234.302087673290197651800289219014f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=2130879087.612024298178862637758553713488f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=2089016769.876078818357383252778347616041f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=375057949.281131299583940238057282153047f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1277341619.398827626316339650748800401908f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1061633222.835329512461374195361430950942f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1365405289.773869111114948763632675850648f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1961005639.078089952619535750239742111912f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=2112278788.967415797091203424010042934607f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=471280962.805075222117434964070891303658f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1844770510.210029724830200371918073811488f;if (eMiRWcvLrWObfyXZpJmHTlPLthTUmM - eMiRWcvLrWObfyXZpJmHTlPLthTUmM> 0.00000001 ) eMiRWcvLrWObfyXZpJmHTlPLthTUmM=1587960762.267490842524125070204385125721f; else eMiRWcvLrWObfyXZpJmHTlPLthTUmM=906758118.845677481063940506895238509348f;float EMFjqvXasgswHXETUagMuuPhhDjIip=1125176818.618410985458881175474267428250f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=2070117695.991611645101181726000591046110f; else EMFjqvXasgswHXETUagMuuPhhDjIip=531143079.384556828164048328671235041168f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=461932711.553682863600994666473855916952f; else EMFjqvXasgswHXETUagMuuPhhDjIip=1161772600.686162156666517144259580680977f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=413864280.791203493361082735898281311759f; else EMFjqvXasgswHXETUagMuuPhhDjIip=1147963895.229007641576299960895628052090f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=254561743.384425219610432200356106460105f; else EMFjqvXasgswHXETUagMuuPhhDjIip=777615120.565230387576239630685385528816f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=858713396.521415621303399007894152039081f; else EMFjqvXasgswHXETUagMuuPhhDjIip=861852513.268105583411049319653203357058f;if (EMFjqvXasgswHXETUagMuuPhhDjIip - EMFjqvXasgswHXETUagMuuPhhDjIip> 0.00000001 ) EMFjqvXasgswHXETUagMuuPhhDjIip=167718643.765025629453558422473043916696f; else EMFjqvXasgswHXETUagMuuPhhDjIip=2024456041.863586603589130176503668672212f;int BOQguvAjLmIvuyTxJPsSABwPJzbwDU=775820314;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 0 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=1494634144; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=512981449;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 0 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=1804726562; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=1063934303;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 0 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=1930455568; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=7635294;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 1 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=661765008; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=101045639;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 0 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=488728553; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=970728544;if (BOQguvAjLmIvuyTxJPsSABwPJzbwDU == BOQguvAjLmIvuyTxJPsSABwPJzbwDU- 0 ) BOQguvAjLmIvuyTxJPsSABwPJzbwDU=1370808819; else BOQguvAjLmIvuyTxJPsSABwPJzbwDU=545596715;int uYEunYgrKJJEuvNkNaJvayXraCLSyk=430688378;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 0 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=419748587; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=1868511516;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 1 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=925420690; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=1687575147;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 1 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=1890987659; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=1591472072;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 0 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=168674954; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=982915192;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 1 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=1834455026; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=1744926476;if (uYEunYgrKJJEuvNkNaJvayXraCLSyk == uYEunYgrKJJEuvNkNaJvayXraCLSyk- 0 ) uYEunYgrKJJEuvNkNaJvayXraCLSyk=418628223; else uYEunYgrKJJEuvNkNaJvayXraCLSyk=199780709;float AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=461584912.909965199160408119078440064618f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1996216528.845089869314803231528497457000f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=812140448.168243542945177464783410045974f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1559607621.397935676170979342009332786920f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=813189934.868966925290871454181307506526f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1895833167.975731312452892967143284067147f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1798812158.040032625125384961565059370177f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1342204897.527641286044213901121779210286f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=2062482401.356870884378653284758015786944f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=2016861718.211080855313786312713663945219f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=360887072.504126768343054543058534166825f;if (AcBzOiMJCMpOnWrLZymaJDMeHRVnNG - AcBzOiMJCMpOnWrLZymaJDMeHRVnNG> 0.00000001 ) AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=673044047.675782966066447172941227118107f; else AcBzOiMJCMpOnWrLZymaJDMeHRVnNG=1590619383.195387947378709219710599545967f;float shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1041478023.159984208380996262654781809183f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1073240850.157457635196842841961585757829f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1884513912.623533455074867995906822378270f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1604707454.081767442059700782996392380126f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=22370924.628519310066305822269649579949f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1231759007.732653129157394010766996709015f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=308462482.148233576370322345557028395605f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1032316302.422511916264459768557456922095f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=2050645719.998196891143049368074725907770f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=344306495.093247746497741451766513128339f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=821811370.922225181263269709556701469122f;if (shwwkGFdCKQWglQXNrvCqUsnfzfuTH - shwwkGFdCKQWglQXNrvCqUsnfzfuTH> 0.00000001 ) shwwkGFdCKQWglQXNrvCqUsnfzfuTH=1376962924.051757662647061435442088433187f; else shwwkGFdCKQWglQXNrvCqUsnfzfuTH=185529092.717932262325701974113478117443f;double cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=552178955.745657052843699832850746483809;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=749332196.761373136889144379675700701028; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=909223907.536318242770708270211424417267;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=696863834.221836483613700511264182811291; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=572414795.219656097290871883491701325012;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=405111399.629341515562408541104890061933; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=1122905376.218677153137904930399491884939;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=682338858.079236483149917809713164592880; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=768420076.432517878966736068519532601575;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=526306619.471482841003296843964448190002; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=1010421936.640359546532851645786872214439;if (cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc == cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc ) cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=1818197076.285057495841843279865642688814; else cmYtTDEPsLkSIMiLtlbdtwoTPTsrHc=1530100587.621736781525195613005578574447;int fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=2125489998;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 0 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=204382952; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1524633847;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 0 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1274023506; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1460656381;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 0 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=479325791; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=199025424;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 1 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1377309552; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1687754046;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 1 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1927307793; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1656445549;if (fpCKLyHFnbpuPTBcYkAncsFvDZRKiH == fpCKLyHFnbpuPTBcYkAncsFvDZRKiH- 0 ) fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=468883439; else fpCKLyHFnbpuPTBcYkAncsFvDZRKiH=1074021797;long TGjDBZJvccixUGyYJFuSplirTUFQwm=1554877419;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 1 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=1457528079; else TGjDBZJvccixUGyYJFuSplirTUFQwm=120865092;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 1 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=841287187; else TGjDBZJvccixUGyYJFuSplirTUFQwm=1015414816;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 1 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=1159584841; else TGjDBZJvccixUGyYJFuSplirTUFQwm=968128729;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 0 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=636366624; else TGjDBZJvccixUGyYJFuSplirTUFQwm=2097819121;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 1 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=1202175054; else TGjDBZJvccixUGyYJFuSplirTUFQwm=836287378;if (TGjDBZJvccixUGyYJFuSplirTUFQwm == TGjDBZJvccixUGyYJFuSplirTUFQwm- 1 ) TGjDBZJvccixUGyYJFuSplirTUFQwm=1643547952; else TGjDBZJvccixUGyYJFuSplirTUFQwm=221337071;int nhjzagTosgwxLfUNPNxebMgIfwSAjF=1766250924;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 0 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=1763534901; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=1398956573;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 0 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=2128629787; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=211856871;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 1 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=1335444440; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=1737669906;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 1 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=72853164; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=550306772;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 0 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=1679398002; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=1156593435;if (nhjzagTosgwxLfUNPNxebMgIfwSAjF == nhjzagTosgwxLfUNPNxebMgIfwSAjF- 1 ) nhjzagTosgwxLfUNPNxebMgIfwSAjF=792590649; else nhjzagTosgwxLfUNPNxebMgIfwSAjF=896149497;float QlblsNjWkQejNSCebWIFtJwvDjIiVQ=1517114461.878189021276099026443048944916f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=684430253.465893243990095502535778684034f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=263319657.622681231698366315455248279722f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=72572508.022938952447205742238978059441f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=797562933.351067643956411814353850426509f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=2031735678.961120734811049257725510850265f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=190345300.225409649228165382137988455237f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=965905157.057969664072851756558814935706f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=1383737870.541237220682926523880078025414f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=888267223.506963263564608600842203668331f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=1323688032.381728717346142747052567231331f;if (QlblsNjWkQejNSCebWIFtJwvDjIiVQ - QlblsNjWkQejNSCebWIFtJwvDjIiVQ> 0.00000001 ) QlblsNjWkQejNSCebWIFtJwvDjIiVQ=258481327.694229408190461626202139982459f; else QlblsNjWkQejNSCebWIFtJwvDjIiVQ=1790522060.848685392444906321762524957635f;long yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=277608431;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 1 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=524227855; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=850116162;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 0 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=1457970543; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=1170376308;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 1 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=1175529665; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=114134812;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 0 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=219432979; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=1867341820;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 1 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=48011928; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=290980415;if (yWvQdJlpgTOYndpIPPhcFlwoNHUoNr == yWvQdJlpgTOYndpIPPhcFlwoNHUoNr- 1 ) yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=1483500461; else yWvQdJlpgTOYndpIPPhcFlwoNHUoNr=2991406;int KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1593857993;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 0 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=489716536; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=126433491;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 0 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1684390465; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=374429396;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 0 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1577126854; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=334654761;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 1 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1881998893; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1573189601;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 0 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=433051416; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=744891917;if (KOJYwkNfvvyBCHJQGczWVjIeZhtrBu == KOJYwkNfvvyBCHJQGczWVjIeZhtrBu- 0 ) KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=1471073295; else KOJYwkNfvvyBCHJQGczWVjIeZhtrBu=11756408;double lSLgxhaUOzwEfrfffzZZNGdHDBmDta=523425521.408147962801639227059474337322;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=106588442.209107227593105576176345803482; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=514982218.075161254310057647996126797440;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=371921642.484653905533396649927660695058; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=1951389422.492494650574347494016852492507;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=297830165.806714291309955433420204275011; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=1323678181.225478863431382790706028771365;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=341584796.928119569867010093358531530770; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=570295441.761951338334205902979911850768;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=43886678.989921726086603444773991296121; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=352714335.168046638539690431841710619883;if (lSLgxhaUOzwEfrfffzZZNGdHDBmDta == lSLgxhaUOzwEfrfffzZZNGdHDBmDta ) lSLgxhaUOzwEfrfffzZZNGdHDBmDta=749551005.571692877891624530196638056943; else lSLgxhaUOzwEfrfffzZZNGdHDBmDta=1740755263.785607811867516029629080704375;long wLRFCgyKohLvSOuFJLzOiPOWkCasBL=1148050096;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 0 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=2027374495; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=812106981;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 1 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=1300805524; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=576527628;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 0 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=1619065663; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=675575597;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 0 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=847965224; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=189573094;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 1 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=1115600916; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=2032264589;if (wLRFCgyKohLvSOuFJLzOiPOWkCasBL == wLRFCgyKohLvSOuFJLzOiPOWkCasBL- 1 ) wLRFCgyKohLvSOuFJLzOiPOWkCasBL=737324892; else wLRFCgyKohLvSOuFJLzOiPOWkCasBL=430637748;float nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1765680824.920992641600514218074723682931f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=443634762.434418150103423819109463630741f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=960074940.072127610726898131198494208289f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=2043886611.842923240720049467560076479993f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1808039509.943108508600157845694183988655f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1515779132.535788895197006192073430087032f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1639891409.095317648906151729889679311580f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1296027828.297321279965517839037935343611f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1313522002.717972994155093526359256347208f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1932332342.877936974595508294393746356264f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1114095024.689639927865287486826302623178f;if (nEFaSloKDVbgWsPYBAsQMoljtDwQnc - nEFaSloKDVbgWsPYBAsQMoljtDwQnc> 0.00000001 ) nEFaSloKDVbgWsPYBAsQMoljtDwQnc=330542718.082102798947853856998145949711f; else nEFaSloKDVbgWsPYBAsQMoljtDwQnc=1224412519.686989429679898965988738104837f;long zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1617621312;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 1 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1684175774; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=323023199;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 1 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=131865560; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=667081598;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 0 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1276492874; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1418301672;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 1 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1051843505; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1578496492;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 1 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=1814500641; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=182348548;if (zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ == zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ- 0 ) zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=835788506; else zqkPdJvjVWRnSKvOZhzoToPRvcmnuZ=413049223;int rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1983693754;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 1 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=2112044741; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1131353828;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 1 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1297120501; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=2041737193;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 0 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1066510787; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1547643928;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 1 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1779754611; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=387483086;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 1 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=1663562800; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=2134767169;if (rzEZpnnjvfnNePgKFYFZivGOXCcjCv == rzEZpnnjvfnNePgKFYFZivGOXCcjCv- 1 ) rzEZpnnjvfnNePgKFYFZivGOXCcjCv=791678933; else rzEZpnnjvfnNePgKFYFZivGOXCcjCv=504970639;float ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=974357286.570697179630041325394039380217f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=1313403474.763391603391019239911899655132f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=1267187227.807297885183301402125231797475f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=893945621.757260593356494960935311877660f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=467521965.041153614102098985894865044750f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=75811011.493332645135793443499398738721f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=1868555290.007472076745076928271136220671f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=489187950.492414832247786532281056197236f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=1172816737.240635377210713446180849055742f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=2055715082.870701826126618244375126640168f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=465390119.173544011273006434282147876417f;if (ynMwcPGCTUGRTEWhOOEwYzVvVdALuO - ynMwcPGCTUGRTEWhOOEwYzVvVdALuO> 0.00000001 ) ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=2141022515.276816427423421322032709703788f; else ynMwcPGCTUGRTEWhOOEwYzVvVdALuO=1333430584.960511730111786983756459568821f;double kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=2021385294.784865536739349314256159868296;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=1334344747.526723230371416544954430373601; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=494332611.735022463628499584342444329746;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=1259851425.073064740418233614180937940181; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=601233575.483876554594594421172566714713;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=991606776.567914647826170655452902002726; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=824607569.295103868098446659717946177502;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=551110525.393264207799747457746311945914; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=136531846.737781638255303930082196709343;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=690752235.930417207692050111912503264218; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=1891866121.340543690673440025660844253725;if (kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO == kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO ) kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=1151002445.310046352862234027425065950739; else kWyYfQtCBfsJuKFwHLqLyzNHbJHzdO=1289209352.098537340487391871033731045610;long iJNnjWfAmioizWthkVEqviCfReibwF=43464289;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 0 ) iJNnjWfAmioizWthkVEqviCfReibwF=1725362857; else iJNnjWfAmioizWthkVEqviCfReibwF=2008914867;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 1 ) iJNnjWfAmioizWthkVEqviCfReibwF=1247129396; else iJNnjWfAmioizWthkVEqviCfReibwF=1466649709;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 0 ) iJNnjWfAmioizWthkVEqviCfReibwF=1164485074; else iJNnjWfAmioizWthkVEqviCfReibwF=1459711959;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 0 ) iJNnjWfAmioizWthkVEqviCfReibwF=1907882743; else iJNnjWfAmioizWthkVEqviCfReibwF=243489451;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 0 ) iJNnjWfAmioizWthkVEqviCfReibwF=642165878; else iJNnjWfAmioizWthkVEqviCfReibwF=124303764;if (iJNnjWfAmioizWthkVEqviCfReibwF == iJNnjWfAmioizWthkVEqviCfReibwF- 1 ) iJNnjWfAmioizWthkVEqviCfReibwF=2133505758; else iJNnjWfAmioizWthkVEqviCfReibwF=601120668;float pabxzYQDkltfGFfqpkyCBWFtXygeLL=780584195.758731644044627809326686751068f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=1407598456.923822596395411635600882218208f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=917777286.179623181631313237523646914262f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=1318109539.440832780955507957919279653381f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=1998183915.993930269155877915007745337138f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=511184927.051917278870986551375948526861f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=3235763.314221406175723897998696411340f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=2130234157.141866786615621022583908373882f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=488795905.016703507822248318649097735469f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=716076702.490460093863686286006635413018f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=757413775.815843289942589641155056397615f;if (pabxzYQDkltfGFfqpkyCBWFtXygeLL - pabxzYQDkltfGFfqpkyCBWFtXygeLL> 0.00000001 ) pabxzYQDkltfGFfqpkyCBWFtXygeLL=1924253377.819605985410816515788396397956f; else pabxzYQDkltfGFfqpkyCBWFtXygeLL=331428537.768424409486785634198304852432f;float yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1508842216.268027012651188230200846337912f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1379410153.875763751005872710159020053380f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=2097558226.111198742558920608463797496681f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1755551374.158290588517871110156593743580f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=942671683.327240184013005702639654667556f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1783332856.701820063213714360915195287479f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1408002651.463842328674053695131775272675f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=856900220.470585008214234125369244870977f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=2022116435.076431567860892754116831724672f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=733933515.623124991476851955222686234681f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=1016361974.899635787987385009188945004890f;if (yPukXNRzNDGeMwHfGGlfTrhlUhSfhH - yPukXNRzNDGeMwHfGGlfTrhlUhSfhH> 0.00000001 ) yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=2106551344.142068822637944631134660405464f; else yPukXNRzNDGeMwHfGGlfTrhlUhSfhH=867303012.109885533728756111248477659277f;int SfrbOrFevExpWDdrDXPvWNKfiMLGQg=244428208;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 1 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=161002291; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=370418250;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 1 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=2029094543; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=1856329100;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 1 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=1376246974; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=1382763345;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 0 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=1679763009; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=1722127094;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 0 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=2026164357; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=758333491;if (SfrbOrFevExpWDdrDXPvWNKfiMLGQg == SfrbOrFevExpWDdrDXPvWNKfiMLGQg- 0 ) SfrbOrFevExpWDdrDXPvWNKfiMLGQg=750573007; else SfrbOrFevExpWDdrDXPvWNKfiMLGQg=407071299;long GOLsChzRHabGBdIbbsesQADIqEurPa=1442187280;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 0 ) GOLsChzRHabGBdIbbsesQADIqEurPa=1725593413; else GOLsChzRHabGBdIbbsesQADIqEurPa=624663542;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 1 ) GOLsChzRHabGBdIbbsesQADIqEurPa=1369194162; else GOLsChzRHabGBdIbbsesQADIqEurPa=1604030481;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 1 ) GOLsChzRHabGBdIbbsesQADIqEurPa=838256300; else GOLsChzRHabGBdIbbsesQADIqEurPa=788192147;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 0 ) GOLsChzRHabGBdIbbsesQADIqEurPa=1144734986; else GOLsChzRHabGBdIbbsesQADIqEurPa=2147079452;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 0 ) GOLsChzRHabGBdIbbsesQADIqEurPa=60877066; else GOLsChzRHabGBdIbbsesQADIqEurPa=1443476751;if (GOLsChzRHabGBdIbbsesQADIqEurPa == GOLsChzRHabGBdIbbsesQADIqEurPa- 1 ) GOLsChzRHabGBdIbbsesQADIqEurPa=1264250400; else GOLsChzRHabGBdIbbsesQADIqEurPa=1642306600;long hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=766086845;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 0 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=27503342; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=1679825169;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 0 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=170426246; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=347134595;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 1 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=1465851349; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=837348545;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 1 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=132595242; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=2144130455;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 0 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=417795217; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=33424280;if (hPTOSPcjiOHIdsaPaXEuryQEQoVVvc == hPTOSPcjiOHIdsaPaXEuryQEQoVVvc- 0 ) hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=1063990973; else hPTOSPcjiOHIdsaPaXEuryQEQoVVvc=1024999365; }
 hPTOSPcjiOHIdsaPaXEuryQEQoVVvcy::hPTOSPcjiOHIdsaPaXEuryQEQoVVvcy()
 { this->besOsTkgwpmt("CROBZcWQAFtobvnIgjMAtEptiyOxdtbesOsTkgwpmtj", true, 657429644, 449828921, 339174206); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xMMxcrRtvvaJGEagFhCNSWMxiIxdnWy
 { 
public: bool ehPYMLbuEsnjjGCJtHINOdWsazPOTu; double ehPYMLbuEsnjjGCJtHINOdWsazPOTuxMMxcrRtvvaJGEagFhCNSWMxiIxdnW; xMMxcrRtvvaJGEagFhCNSWMxiIxdnWy(); void oFUcrfsSviLm(string ehPYMLbuEsnjjGCJtHINOdWsazPOTuoFUcrfsSviLm, bool nGYewbrgOUenxUfncrgzDWeJCALWxM, int wMbKEhLFJaSNsnibfBNnXYiUqQbObv, float QUiBYSLnPxcWhGgpdrbbVJIKpXdGTn, long GweOKxerOQPrlQKArcnAnjnAZHMXLB);
 protected: bool ehPYMLbuEsnjjGCJtHINOdWsazPOTuo; double ehPYMLbuEsnjjGCJtHINOdWsazPOTuxMMxcrRtvvaJGEagFhCNSWMxiIxdnWf; void oFUcrfsSviLmu(string ehPYMLbuEsnjjGCJtHINOdWsazPOTuoFUcrfsSviLmg, bool nGYewbrgOUenxUfncrgzDWeJCALWxMe, int wMbKEhLFJaSNsnibfBNnXYiUqQbObvr, float QUiBYSLnPxcWhGgpdrbbVJIKpXdGTnw, long GweOKxerOQPrlQKArcnAnjnAZHMXLBn);
 private: bool ehPYMLbuEsnjjGCJtHINOdWsazPOTunGYewbrgOUenxUfncrgzDWeJCALWxM; double ehPYMLbuEsnjjGCJtHINOdWsazPOTuQUiBYSLnPxcWhGgpdrbbVJIKpXdGTnxMMxcrRtvvaJGEagFhCNSWMxiIxdnW;
 void oFUcrfsSviLmv(string nGYewbrgOUenxUfncrgzDWeJCALWxMoFUcrfsSviLm, bool nGYewbrgOUenxUfncrgzDWeJCALWxMwMbKEhLFJaSNsnibfBNnXYiUqQbObv, int wMbKEhLFJaSNsnibfBNnXYiUqQbObvehPYMLbuEsnjjGCJtHINOdWsazPOTu, float QUiBYSLnPxcWhGgpdrbbVJIKpXdGTnGweOKxerOQPrlQKArcnAnjnAZHMXLB, long GweOKxerOQPrlQKArcnAnjnAZHMXLBnGYewbrgOUenxUfncrgzDWeJCALWxM); };
 void xMMxcrRtvvaJGEagFhCNSWMxiIxdnWy::oFUcrfsSviLm(string ehPYMLbuEsnjjGCJtHINOdWsazPOTuoFUcrfsSviLm, bool nGYewbrgOUenxUfncrgzDWeJCALWxM, int wMbKEhLFJaSNsnibfBNnXYiUqQbObv, float QUiBYSLnPxcWhGgpdrbbVJIKpXdGTn, long GweOKxerOQPrlQKArcnAnjnAZHMXLB)
 { double FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=597499717.061515520927864257099992813583;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1339835090.143508373972640159518248318981; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1436831020.214889446708523835377408591639;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1585709306.049377887281482082659471491730; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1429402565.882776926706920703761908177998;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1406394750.626740082616706466289908909561; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=679565126.914819028183115631281282082985;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=1449420857.252271399845810689348838084293; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=379987348.724403205534355747341365853539;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=928346773.720712152859592469435806346075; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=967771726.177681464370559298340410503558;if (FXpNJFIdDIUqNdDlAlItiJKlyKNfwj == FXpNJFIdDIUqNdDlAlItiJKlyKNfwj ) FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=493109006.719778291110617545716973677819; else FXpNJFIdDIUqNdDlAlItiJKlyKNfwj=365205515.992676288110834091416984294611;long WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=1609328374;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 1 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=337778800; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=1449613645;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 1 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=541186412; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=566360728;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 1 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=239096964; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=2124240693;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 1 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=2058965078; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=1543345835;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 1 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=1115699852; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=2057215850;if (WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ == WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ- 0 ) WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=949532263; else WsJKFqwWCTegfBeFXwhrdTBNAHnVqJ=1275183153;double xQXqTlzubsFVdaApNjrJfbofBcEgIu=2109210271.968808077911167088742694364993;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=456873557.483039395999460604917909438455; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=1982820723.518463823879396423476018758560;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=92715870.294049111889215872312509060171; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=1533762627.546378635832279781409726979619;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=26222759.914145394734768034138078292150; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=98306676.003793823450887607894668205796;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=1512704913.435473117532179939179636645550; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=276806698.924812541559037359047663790984;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=512825777.877772847749872161030762336222; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=1950017047.376879337662330990173006422544;if (xQXqTlzubsFVdaApNjrJfbofBcEgIu == xQXqTlzubsFVdaApNjrJfbofBcEgIu ) xQXqTlzubsFVdaApNjrJfbofBcEgIu=123528669.672548400484448953255616047156; else xQXqTlzubsFVdaApNjrJfbofBcEgIu=1295469901.789409089359089066868030917751;int RcYizXBXvtgStCgePizoDOvnnZxYwK=678332746;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 0 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=1808621376; else RcYizXBXvtgStCgePizoDOvnnZxYwK=1273514045;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 1 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=199126062; else RcYizXBXvtgStCgePizoDOvnnZxYwK=18875167;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 0 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=2017458957; else RcYizXBXvtgStCgePizoDOvnnZxYwK=1463723250;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 0 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=358402753; else RcYizXBXvtgStCgePizoDOvnnZxYwK=1363078044;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 1 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=1525349589; else RcYizXBXvtgStCgePizoDOvnnZxYwK=42363471;if (RcYizXBXvtgStCgePizoDOvnnZxYwK == RcYizXBXvtgStCgePizoDOvnnZxYwK- 0 ) RcYizXBXvtgStCgePizoDOvnnZxYwK=313702713; else RcYizXBXvtgStCgePizoDOvnnZxYwK=1496662547;int GdyoSloFcVlJgribQcJAClaYNGpraw=1543529636;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 1 ) GdyoSloFcVlJgribQcJAClaYNGpraw=2032522286; else GdyoSloFcVlJgribQcJAClaYNGpraw=531382382;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 1 ) GdyoSloFcVlJgribQcJAClaYNGpraw=2055815605; else GdyoSloFcVlJgribQcJAClaYNGpraw=298639190;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 1 ) GdyoSloFcVlJgribQcJAClaYNGpraw=1793569907; else GdyoSloFcVlJgribQcJAClaYNGpraw=1456048882;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 1 ) GdyoSloFcVlJgribQcJAClaYNGpraw=1583105615; else GdyoSloFcVlJgribQcJAClaYNGpraw=239472124;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 1 ) GdyoSloFcVlJgribQcJAClaYNGpraw=2084392379; else GdyoSloFcVlJgribQcJAClaYNGpraw=264379714;if (GdyoSloFcVlJgribQcJAClaYNGpraw == GdyoSloFcVlJgribQcJAClaYNGpraw- 0 ) GdyoSloFcVlJgribQcJAClaYNGpraw=53534951; else GdyoSloFcVlJgribQcJAClaYNGpraw=436563564;long JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1845379469;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 0 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=155194242; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=271199517;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 0 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1614045424; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=63098824;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 1 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1333174722; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1063857920;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 0 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=91751314; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1140633954;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 1 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=1624417970; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=877121473;if (JPVrtwOgsonfbvMfPEqRGFjdmuWDOa == JPVrtwOgsonfbvMfPEqRGFjdmuWDOa- 1 ) JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=8413038; else JPVrtwOgsonfbvMfPEqRGFjdmuWDOa=2131342935;double LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=338588073.961333109266884086452809747824;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=759158921.658604744762346391530846017269; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=727482680.739561929035370056346649592379;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1410431262.858503498658250667617103419018; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1394748753.065139787767980950994677376610;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1509472072.448070323208095032367023540996; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1603382418.317832092164889668245446453663;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1032246486.043709127304130435498286298259; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=352572494.054913118165204131712927597851;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1837892077.208846133732744419227855813753; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=2107137585.737896756801908139329864455413;if (LRZxoSVqNThiAxLWnBZJAZuzqvVIWL == LRZxoSVqNThiAxLWnBZJAZuzqvVIWL ) LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=81966435.448111695886352696779444597445; else LRZxoSVqNThiAxLWnBZJAZuzqvVIWL=1753079243.244456390765464362296974500572;double IWBeVKvweMEgAKUAsvJPZKbuOhCude=761854412.738654988771507589051916462274;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=425148771.338162921365337722437665992239; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=615806891.371455355667979668075185787437;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=1341468305.089210599258897179633426539777; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=1606316993.993869887927093730871880466479;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=707675927.182236614930381754380091611340; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=1127759302.351985696823604261082319920263;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=210354914.547569940716867840756046804207; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=968664366.643439624632237756329052750277;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=439631068.685428453922612591925808869278; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=915181651.946816892031351754037046288767;if (IWBeVKvweMEgAKUAsvJPZKbuOhCude == IWBeVKvweMEgAKUAsvJPZKbuOhCude ) IWBeVKvweMEgAKUAsvJPZKbuOhCude=821704867.938608781781099556821525291073; else IWBeVKvweMEgAKUAsvJPZKbuOhCude=916448434.813621385307446615774394641451;double jHsuipVnnKmCbKWJpwcseQzvYIuynA=2068350013.027132456550910139228035152653;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=1865089526.045741880484817379299872500318; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=1862430525.081452627036985433989346437324;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=1824888290.168250356575237490718843985613; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=1273229344.631407974529752512004762732538;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=1500547562.684394594062730020257371907087; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=369949594.545794546813804314272349132450;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=335907397.752763804668444557879481531667; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=699295471.320716115261162590679569148233;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=1386436678.325834874844361842914110896268; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=1261472930.909204734872783520840632922505;if (jHsuipVnnKmCbKWJpwcseQzvYIuynA == jHsuipVnnKmCbKWJpwcseQzvYIuynA ) jHsuipVnnKmCbKWJpwcseQzvYIuynA=372690394.972605617821914353848653837848; else jHsuipVnnKmCbKWJpwcseQzvYIuynA=1373520784.234001119801402082877986743997;int omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1702578781;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 0 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1514755453; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1394042273;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 0 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1706194164; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1316280719;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 1 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=822057708; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1087940489;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 1 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1778395793; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=1778883266;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 1 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=517127766; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=441766896;if (omwevlwJPGXYxXjMVEpaZvlDOYoMfs == omwevlwJPGXYxXjMVEpaZvlDOYoMfs- 0 ) omwevlwJPGXYxXjMVEpaZvlDOYoMfs=955117685; else omwevlwJPGXYxXjMVEpaZvlDOYoMfs=594290421;double LOBOdsfbWQazdxveGswxRxGSRUkMkO=238645750.809444214657801175275587352910;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=1652269424.379685575951089035066403432220; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=161100069.230438922671032560932806598626;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=2035473364.346572276111340547108339953505; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=1695241421.130734666149583257605163811873;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=244434546.978118171699107996671790008893; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=236620323.499427248519602706793620239821;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=1408790497.148931089075827329831211198920; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=55199177.942858188273883891401019093895;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=279899494.326023407925277954941863994396; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=642172834.649396786049469794324905567413;if (LOBOdsfbWQazdxveGswxRxGSRUkMkO == LOBOdsfbWQazdxveGswxRxGSRUkMkO ) LOBOdsfbWQazdxveGswxRxGSRUkMkO=219880315.912249699227890240431117274493; else LOBOdsfbWQazdxveGswxRxGSRUkMkO=1593686644.048414833110511711643709390507;float HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=2100958997.803874216679706973174795494847f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1184735996.023234991026723415818762811948f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1266609733.148075768308215917319014947152f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1549176628.416409986885862055890474924514f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=53593571.961690392714254425309346215358f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=2040535810.658302427277319732060931563649f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1286071089.515427796301914313869339972794f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1246292305.098887290304786013836099139665f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=777149037.511232314812367575407006811085f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=84034732.864680465570913473277436951543f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=46005024.134905988811093710354881944884f;if (HDhKmxYrfUHnRHFfzdacOsZAOUvFLT - HDhKmxYrfUHnRHFfzdacOsZAOUvFLT> 0.00000001 ) HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=756101578.992344711869656655324230046306f; else HDhKmxYrfUHnRHFfzdacOsZAOUvFLT=1058780666.221593085001969533540056087241f;double TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1270638626.363600704035663875760086564282;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=209052633.863663908493720947245239739725; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=134044644.653228300008838746773814874403;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1868735007.528494051264725850200416385956; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=820791416.500868044479661929002421421311;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=598166001.011587407781793196547605428055; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=182687881.873456918552342284834698442626;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1458144235.055202835838819547435433229601; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1278135130.549261814145010699900539262319;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=2132735423.424430966884339052373071101101; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1650994987.050682168174412730131015646434;if (TFchUrFgTdkxTLRNjDUFNpgdLiZYFc == TFchUrFgTdkxTLRNjDUFNpgdLiZYFc ) TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=1036381225.996694486800520076958218005614; else TFchUrFgTdkxTLRNjDUFNpgdLiZYFc=179884874.774538893935534269567068679869;double skHBKdyuMKuRAHNklWsTbCYnnJbtev=1680293636.109788630572250544253073211569;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=454994015.382955324093129325714937624657; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=1001642335.728230130532126105509785556534;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=1557038072.278738699030639381162926814389; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=1662551465.460636559951757026737548236293;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=714105071.542168000384231237999866292341; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=998826726.136752069013745432711931478607;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=345593587.686036868037809722286064020538; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=366198335.446244772968489414386653272311;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=1062291411.644169167515132969561736389418; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=1258324327.247143442046787163131385001988;if (skHBKdyuMKuRAHNklWsTbCYnnJbtev == skHBKdyuMKuRAHNklWsTbCYnnJbtev ) skHBKdyuMKuRAHNklWsTbCYnnJbtev=1611206689.186037800063763605923749807214; else skHBKdyuMKuRAHNklWsTbCYnnJbtev=198429522.267407185136428863350706554734;int eucnxfPggdOypvlNNHwRUwiUadEubN=538282518;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 1 ) eucnxfPggdOypvlNNHwRUwiUadEubN=724554914; else eucnxfPggdOypvlNNHwRUwiUadEubN=1096725336;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 1 ) eucnxfPggdOypvlNNHwRUwiUadEubN=1384634011; else eucnxfPggdOypvlNNHwRUwiUadEubN=621024264;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 1 ) eucnxfPggdOypvlNNHwRUwiUadEubN=1263066417; else eucnxfPggdOypvlNNHwRUwiUadEubN=1313465112;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 0 ) eucnxfPggdOypvlNNHwRUwiUadEubN=1502792996; else eucnxfPggdOypvlNNHwRUwiUadEubN=1002048115;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 0 ) eucnxfPggdOypvlNNHwRUwiUadEubN=1955949145; else eucnxfPggdOypvlNNHwRUwiUadEubN=271041498;if (eucnxfPggdOypvlNNHwRUwiUadEubN == eucnxfPggdOypvlNNHwRUwiUadEubN- 0 ) eucnxfPggdOypvlNNHwRUwiUadEubN=68341795; else eucnxfPggdOypvlNNHwRUwiUadEubN=389540823;long fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=2046809230;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 0 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1169674313; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1223291589;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 1 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=603786651; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=560673221;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 1 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=332062551; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=402550102;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 1 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=556533555; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1357709554;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 1 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1935934704; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1502536672;if (fgPSZqpwlQLCoLPKXqRBYbCRxQRJay == fgPSZqpwlQLCoLPKXqRBYbCRxQRJay- 1 ) fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1905983652; else fgPSZqpwlQLCoLPKXqRBYbCRxQRJay=1487325321;float HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1782725592.496233320747697875108458447128f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1010627719.641135295411223841279144905889f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=498098707.720311767379080734670706961723f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1602813607.151679191218491127528122903546f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1918818485.105422841164346331595430603088f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=947558785.766621318938746499678215178683f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=402790094.097350973503477789431441156156f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=417227219.483595240652727594959926266630f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1289012550.593722099355089082567331857843f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1646332986.071027001650670404839428426307f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=554989957.536649506219291787146311566361f;if (HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL - HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL> 0.00000001 ) HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=1854085967.171412503100863023785027233518f; else HTGLSnmFWcRmjyeAKTVgqdqiwsIGQL=443063573.425614989028405655954379615439f;int AucnmcLYsIVUlclhLIfmqdeTDaeAzt=2143367627;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 1 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=1349235464; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=1711881106;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 1 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=1176238856; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=404710803;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 1 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=1893706827; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=79694047;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 0 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=206219967; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=322004812;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 0 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=540191781; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=26924457;if (AucnmcLYsIVUlclhLIfmqdeTDaeAzt == AucnmcLYsIVUlclhLIfmqdeTDaeAzt- 1 ) AucnmcLYsIVUlclhLIfmqdeTDaeAzt=832573207; else AucnmcLYsIVUlclhLIfmqdeTDaeAzt=1908013392;long IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=696234432;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 1 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=620908760; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=433099850;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 1 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1526396751; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1899074804;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 0 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1611077471; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=137754025;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 0 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1099250445; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=766884219;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 0 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=806064370; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1462257748;if (IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX == IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX- 1 ) IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1061823882; else IZGeKIdjwDOJZpyLiSmBfiJXLDQRSX=1924556241;float OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1979881940.764163112213538855844981748476f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1386028201.937208207117696431204726689353f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1910099672.575748634065874830631837222162f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=138089857.006574306890725905395983285852f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1154567380.768217041054485111343797812349f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=83475857.956424352896535251917562017150f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1392905582.671220816296055726366153350542f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=2036502412.731223632545583161749289553808f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=930933672.148556683319593278029980465288f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=291878740.094541703768854070749688926820f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1280808795.241633615257537223749565051662f;if (OrVMqCVoKiReJkoESBAbZhoRsxRKoj - OrVMqCVoKiReJkoESBAbZhoRsxRKoj> 0.00000001 ) OrVMqCVoKiReJkoESBAbZhoRsxRKoj=1378626704.509362833147659215271419189769f; else OrVMqCVoKiReJkoESBAbZhoRsxRKoj=920634328.305486928650170302694650129138f;double EgSqVKKjSRtkdNILZrtomFoZkbbMfj=1630865311.515344886030996847191211062494;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=271906681.226676884769170132878702841538; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=1157851535.250292433116454224204073752494;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=1969638460.535751099871076345962844539765; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=497385081.114078171333644310751753270233;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=577139660.334576710937574030122912446290; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=364917314.995800017870093429025012152314;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=532290156.163014387412345393902727699386; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=1211481439.871472808025362378796042244652;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=612630661.500164207450509756642270194923; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=409354637.826752621828134654450811044118;if (EgSqVKKjSRtkdNILZrtomFoZkbbMfj == EgSqVKKjSRtkdNILZrtomFoZkbbMfj ) EgSqVKKjSRtkdNILZrtomFoZkbbMfj=1746130080.138464496723683514654067411571; else EgSqVKKjSRtkdNILZrtomFoZkbbMfj=431449079.952379178037974833964056428116;double sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=426721799.633587210470165891042814021254;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1747183481.489735739740171552492196082966; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1000174914.717619254127240285458086239375;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=521985191.497303457241848606691761983258; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1792349082.198974094793922825874479573653;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=2024861465.131925307571201994026949483051; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1275999543.145276300752830428868073739364;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=612758575.336754703579437511906529318255; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1375486825.705753628782540064722846881854;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=544081085.766987588509423519946453306998; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=595463079.288512767468723453347301931379;if (sKbnlGNGUUUhfLoGsSgppiDkvxcyaG == sKbnlGNGUUUhfLoGsSgppiDkvxcyaG ) sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=1607196064.160132292286368874105475109768; else sKbnlGNGUUUhfLoGsSgppiDkvxcyaG=330268676.259830960226127204416838589513;int aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=149366896;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 1 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1728210305; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1578442218;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 1 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1652649560; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1685579894;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 1 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=291315169; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=2077323189;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 0 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1402742280; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1021110887;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 1 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1377809516; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1074092065;if (aQUzQhYQrwHTtnrQaqtzZRbHUMERbE == aQUzQhYQrwHTtnrQaqtzZRbHUMERbE- 1 ) aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=541936719; else aQUzQhYQrwHTtnrQaqtzZRbHUMERbE=1821604867;int nMpWDKOQFzPQlGKOmWackCTxbXDYqv=2010215014;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 0 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=1654751765; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=951904905;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 1 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=1320934494; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=717525620;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 0 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=134712283; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=89182008;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 1 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=1753487493; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=1143390785;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 1 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=545092960; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=594151635;if (nMpWDKOQFzPQlGKOmWackCTxbXDYqv == nMpWDKOQFzPQlGKOmWackCTxbXDYqv- 1 ) nMpWDKOQFzPQlGKOmWackCTxbXDYqv=2100787643; else nMpWDKOQFzPQlGKOmWackCTxbXDYqv=2129160228;long HyQoevPKDsqYlOhMajvhYfYIrbyAKK=434014544;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 1 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1360170952; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1596763184;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 0 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=850722421; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1734395241;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 1 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=923981460; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=87960783;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 1 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=135406630; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1817343939;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 1 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1744916281; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=1648357951;if (HyQoevPKDsqYlOhMajvhYfYIrbyAKK == HyQoevPKDsqYlOhMajvhYfYIrbyAKK- 1 ) HyQoevPKDsqYlOhMajvhYfYIrbyAKK=414539566; else HyQoevPKDsqYlOhMajvhYfYIrbyAKK=950769400;float zCiYpNuuQldgqGYYISaadYctqzFwzz=1086511399.761689534407552924625953920409f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=829883219.862331199088196497899762982887f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=1744464697.913674532283664886853462020046f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=823000558.942228563297571611820613465165f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=2102189710.569735245733467790251023643096f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=1005165270.441884885757629835688602454720f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=479141712.226940268391550441491842994043f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=14654613.127654289067740815212444958505f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=1639028297.632637401637243511328984278471f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=1972438372.688454521341579521154828876946f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=509258775.014115989000019852675833796178f;if (zCiYpNuuQldgqGYYISaadYctqzFwzz - zCiYpNuuQldgqGYYISaadYctqzFwzz> 0.00000001 ) zCiYpNuuQldgqGYYISaadYctqzFwzz=1140486934.878634746474226592003538128243f; else zCiYpNuuQldgqGYYISaadYctqzFwzz=1844647181.982223248511620172731518645152f;float bASatNBAQNVXXrJtEnychZEKWlVfop=1175788036.564060074882065528677920710208f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=769221594.275309809120922483132494365446f; else bASatNBAQNVXXrJtEnychZEKWlVfop=107922175.198761886673066710833717699114f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=461433915.457414336261757043966661348118f; else bASatNBAQNVXXrJtEnychZEKWlVfop=197495965.388233676248478316175635989718f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=754330912.732999353548882830303205476445f; else bASatNBAQNVXXrJtEnychZEKWlVfop=178668266.406962768016473896316041553753f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=1086233554.301915292127240894278114956651f; else bASatNBAQNVXXrJtEnychZEKWlVfop=1566790982.001671566961439091186069530807f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=816498275.965112667934831878507558605324f; else bASatNBAQNVXXrJtEnychZEKWlVfop=1651074202.286010632843535333877230042862f;if (bASatNBAQNVXXrJtEnychZEKWlVfop - bASatNBAQNVXXrJtEnychZEKWlVfop> 0.00000001 ) bASatNBAQNVXXrJtEnychZEKWlVfop=1120092986.835486883225787513348365815717f; else bASatNBAQNVXXrJtEnychZEKWlVfop=633837979.998101825056075266464053568068f;double IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1753939252.011505288607900087616129639112;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1012923959.586251399830188095833850790207; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1014276244.618623275970889705275538983766;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1299277009.220322367692097418100503329074; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1308223847.052076056754879354268666772711;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1526504569.430364012246256684507591750731; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=350164408.445642561635302252110831405962;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1576332921.448281161412407339060973171435; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=881029240.871920713119171954518690258622;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1582108571.084764158872795190222803739187; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1359177771.219124642781674601617940038104;if (IjePPlgobfxpRuJLxzcZwMZRvWZdYS == IjePPlgobfxpRuJLxzcZwMZRvWZdYS ) IjePPlgobfxpRuJLxzcZwMZRvWZdYS=1909440516.608177368166964233924867147010; else IjePPlgobfxpRuJLxzcZwMZRvWZdYS=414722685.654818351834456218691639095114;float RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=323850582.333684887143848592566162807390f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1532451961.845193361292502559770829138289f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1386235177.859830986523238577036963013617f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=181547806.629255387277335769176225409783f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=626140649.838091509420097964835464151495f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=477203440.670555034536185724954595858405f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1531215358.012586054293140948044864128680f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=332180487.864053570087077256592056235847f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=651214953.858454929848817790808319992983f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=658231143.983423962334786525955504064067f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1403693223.214707549583858463457814848057f;if (RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ - RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ> 0.00000001 ) RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1285691435.901623019918658286760161050196f; else RZqbBJBbftlLHtyWEFuPtTNuDOYpLQ=1501574715.950369324655555027677340784077f;int xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1169720279;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 1 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=99930814; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1534031329;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 0 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=831723222; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1109742949;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 0 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=121788690; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=955165790;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 1 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1796767114; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=419057186;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 0 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=679072901; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1727888322;if (xMMxcrRtvvaJGEagFhCNSWMxiIxdnW == xMMxcrRtvvaJGEagFhCNSWMxiIxdnW- 1 ) xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=762871041; else xMMxcrRtvvaJGEagFhCNSWMxiIxdnW=1542636788; }
 xMMxcrRtvvaJGEagFhCNSWMxiIxdnWy::xMMxcrRtvvaJGEagFhCNSWMxiIxdnWy()
 { this->oFUcrfsSviLm("ehPYMLbuEsnjjGCJtHINOdWsazPOTuoFUcrfsSviLmj", true, 416711397, 671510869, 471833486); }
#pragma optimize("", off)
 // <delete/>

