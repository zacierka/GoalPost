#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"

#include <nlohmann/json.hpp>
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_BUILD);


using json = nlohmann::json;

class GoalPost: public BakkesMod::Plugin::BakkesModPlugin
{
private:

	bool isCompetitiveMatch(ServerWrapper server);
	bool isLocalPlayerWinner(ServerWrapper server);

	std::string getPlaylistString(int id);

	void collectAndSendMatchStats();
	void sendStats(std::string data);
	void testStats();
	void resetFlags();
	void onOvertime();
	std::string getPlatformString(OnlinePlatform platform);
	std::string getCarString();
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
};
