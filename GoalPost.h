#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"

#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

#include <nlohmann/json.hpp>

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

//using client = websocketpp::client<websocketpp::config::asio_client>;
//using json = nlohmann::json;

class GoalPost: public BakkesMod::Plugin::BakkesModPlugin
{
	//,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window

	std::shared_ptr<bool> enabled;

	//Boilerplate
	void onLoad() override;
	void onUnload() override;

public:
	//void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
private:
	//client ws_client;
	//websocketpp::connection_hdl connection_handle;
	std::string websocket_uri;

	//void wsConnect();
	//void wsDisconnect();
	//void wsSendMessageAsJSON(const json& payload);
};
