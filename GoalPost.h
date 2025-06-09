#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"

#include <nlohmann/json.hpp>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "-" stringify(VERSION_BUILD);


using json = nlohmann::json;

class GoalPost: public BakkesMod::Plugin::BakkesModPlugin, public SettingsWindowBase
{
private:

	bool isCompetitiveMatch(ServerWrapper server);
	bool isLocalPlayerWinner(ServerWrapper server);

	std::string getPlaylistString(int id);

	void collectAndSendMatchStats();
	void sendStats(std::string& data);
	std::string getMapName(std::string internalMap);
	void ping();
	void resetFlags();
	void onOvertime();
	std::string getPlatformString(OnlinePlatform platform);
	void getPlayersStats(ArrayWrapper<PriWrapper> pris, json& players, int playlistId);
	bool isFF(ServerWrapper server);
	bool isOvertime = false;
public:
	//void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
	//public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//public PluginWindowBase // Uncomment if you want to render your own plugin window
	//Boilerplate
	void onLoad() override;
	void onUnload() override;
	void RenderSettings() override;
	const std::map<std::string, std::string> FreeplayMaps{
		{"random",					 "random"},
		{ "CHN_Stadium_Day_P",       "Forbidden Temple (Day)" },
		{ "CHN_Stadium_P",           "Forbidden Temple" },
		{ "EuroStadium_Night_P",     "Mannfield (Night)" },
		{ "EuroStadium_P",           "Mannfield" },
		{ "EuroStadium_Rainy_P",     "Mannfield (Stormy)" },
		{ "Farm_Night_P",            "Farmstead (Night)" },
		{ "HoopsStadium_P",          "Dunk House (Hoops)" },
		{ "hoopsStreet_p",			 "The Block (Hoops"},
		{ "NeoTokyo_Standard_P",     "Neo Tokyo" },
		{ "Park_Night_P",            "Beckwith Park (Midnight)" },
		{ "Park_P",                  "Beckwith Park" },
		{ "Park_Rainy_P",            "Beckwith Park (Stormy)" },
		{ "ShatterShot_P",           "Core 707 (DropShot)" },
		{ "Stadium_Foggy_P",         "DFH Stadium (Stormy)" },
		{ "Stadium_P",               "DFH Stadium" },
		{ "Stadium_Race_Day_p",      "DFH Stadium (Circuit)" },
		{ "Stadium_Winter_P",        "DFH Stadium (Snowy)" },
		{ "street_p",				 "Sovereign Heights" },
		{ "TrainStation_Dawn_P",     "Urban Central (Dawn)" },
		{ "TrainStation_Night_P",    "Urban Central (Night)" },
		{ "TrainStation_P",          "Urban Central" },
		{ "Underwater_P",            "AquaDome" },
		{ "UtopiaStadium_Dusk_P",    "Utopia Coliseum (Dusk)" },
		{ "UtopiaStadium_P",         "Utopia Coliseum" },
		{ "UtopiaStadium_Snow_P",    "Utopia Coliseum (Snowy)" },
		{ "beach_night_p",           "Salty Shores (Night)" },
		{ "cs_day_p",                "Champions Field (Day)" },
		{ "cs_hw_p",                 "Rivals Arena" },
		{ "cs_p",                    "Champions Field" },
		{ "farm_p",                  "Farmstead" },
		{ "music_p",                 "Neon Fields" },
		{ "stadium_day_p",           "DFH Stadium (Day)" },
		{ "throwbackstadium_P",      "Throwback Stadium" },
		{ "wasteland_Night_S_P",     "Wasteland (Night)" },
	};
};
