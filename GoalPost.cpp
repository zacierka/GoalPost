#include "pch.h"
#include "GoalPost.h"


BAKKESMOD_PLUGIN(GoalPost, "Export game events over a websocket", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void GoalPost::onLoad()
{
	_globalCvarManager = cvarManager;
	LOG("Plugin loaded!");
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("GoalPost debug mode enabled");

	cvarManager->registerCvar("websocket_uri", "ws://your-websocket-endpoint.com", "The WebSocket URI to connect to")
		.addOnValueChanged([this](std::string oldValue, CVarWrapper newValue) {
			std::string newUri = newValue.getStringValue();
			std::string prefix = "ws://";
			if (newUri.rfind(prefix, 0) != 0)
			{
				newUri = prefix + newUri;
				newValue.setValue(newUri);
			}
			
			LOG("the cvar websocket_uri changed to {}", newUri);
		});
}

void GoalPost::onUnload()
{
	LOG("Plugin Unloaded");
	//wsDisconnect();
}