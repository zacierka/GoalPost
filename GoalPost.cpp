#include "pch.h"
#include "GoalPost.h"
#include <cpr/cpr.h>
#include <chrono>

#define ON_ALL_TEAMS_CREATED "Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated"
#define ON_POST_BEGIN_PLAY "Function TAGame.Team_TA.PostBeginPlay"
#define ON_OVERTIME_UPDATED "Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated"
#define ON_MATCH_ENDED "Function TAGame.GameEvent_Soccar_TA.OnMatchEnded"
#define ON_MATCH_DESTROYED "Function TAGame.GameEvent_TA.Destroyed"
#define ON_HANDLE_STAT_TICKER "Function TAGame.GFxHUD_TA.HandleStatTickerMessage"
#define ON_MATCH_WINNER_SET "Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet"
#define ON_COUNTDOWN_BEGINSTATE "Function GameEvent_TA.Countdown.BeginState"
BAKKESMOD_PLUGIN(GoalPost, "GoalPost", plugin_version, PLUGINTYPE_FREEPLAY)
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
std::shared_ptr<GameWrapper> _globalGameWrapper;

void GoalPost::onLoad()
{
	_globalCvarManager = cvarManager;
	_globalGameWrapper = gameWrapper;

	// Register CVars for configuration
	cvarManager->registerCvar("goalpost_endpoint", "rl.eboyclique.com:3000", "Endpoint URL for match stats"); // on cvar changed, "ping" website. if fail disable POST.
	cvarManager->registerCvar("goalpost_enabled", "1", "Enable/disable match stats collection", true, true, 0, true, 1);

	gameWrapper->HookEventPost(ON_COUNTDOWN_BEGINSTATE, // works with getonlinegame call but is called every countdown
		[this](std::string eventName) { // find an event that is called at the start of the game once where all players are accessable
			if (!isInGame && cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
				ServerWrapper server = gameWrapper->GetOnlineGame();
				if (!server) {
					LOG("[GOALPOST] Not in an online game!");
					return;
				}

				MMRWrapper mmrw = gameWrapper->GetMMRWrapper();
				int currentPlaylist = mmrw.GetCurrentPlaylist();
				for (int i = 0; i < server.GetPRIs().Count(); i++)
				{
					PriWrapper player = server.GetPRIs().Get(i);
					std::string id = player.GetUniqueIdWrapper().GetIdString();
					Player pl(id, player.GetPlayerName().ToString(), player.GetTeamNum());
					int currentMMR = static_cast<int>(mmrw.GetPlayerMMR(player.GetUniqueIdWrapper(), currentPlaylist));
					SkillRank skillRank = mmrw.GetPlayerRank(player.GetUniqueIdWrapper(), currentPlaylist);
					LOG("id[{}] - MMR[{}] Skill[{}]", id, currentMMR, skillRank.Tier + "div" + skillRank.Division);
					pl.setMMR(currentMMR);
					pl.setRank(skillRank.Tier + "div" + skillRank.Division);
					players.insert({ id, pl });
					LOG("Added Player - {}", player.GetPlayerName().ToString());
				}
				isInGame = true;
			}
			if (isInGame)
			{
				for (auto& [id, player] : players)
				{
					Player* p = &player;
					LOG(p->to_scoreboard());
				}
			}
		});
	// called any time a stat should appear in the center of the screen
	// catches all stats, but only for the primary player
	// gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatEvent", std::bind(&GoalPost::onStatEvent, this, std::placeholders::_1, std::placeholders::_2));
	// called any time a stat should appear on the top right ticker
	// all players
	gameWrapper->HookEventWithCallerPost<ServerWrapper>(ON_HANDLE_STAT_TICKER, std::bind(&GoalPost::onStatTickerEvent, this, std::placeholders::_1, std::placeholders::_2));


	// Game Destroyed Event -> leave early or disconnect
	gameWrapper->HookEventPost(ON_MATCH_DESTROYED,
	[this](std::string eventName) {
		if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
			isOvertime = false; // work on this since disconnects and rejoins
			isInGame = false;
		}
		});

	gameWrapper->HookEventPost(ON_OVERTIME_UPDATED,
		[this](std::string eventName) {
			isOvertime = true;
		});

	// Chat Event -> count and store chats.

	gameWrapper->HookEventPost(ON_MATCH_WINNER_SET,
		[this](std::string eventName) {
			if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {

				ServerWrapper server = gameWrapper->GetOnlineGame();
				if (!server) {
					LOG("[GOALPOST] Not in an online game!");
					return;
				}

				const auto now = std::chrono::system_clock::now();
				std::string datetime = std::format("{:%FT%TZ}", now);

				json results;
				results["score"] = server.GetTotalScore();
				results["orangescore"] = server.GetTeams().Get(0).GetScore();
				results["bluescore"] = server.GetTeams().Get(1).GetScore();
				results["overtime"] = isOvertime;
				results["forfeit"] = (server.GetbForfeit() == 1);

				json match;
				match["matchId"] = server.GetMatchGUID();
				match["datetime"] = datetime;
				match["map"] = getMapName(gameWrapper->GetCurrentMap());
				match["gamemode"] = getPlaylistString(server.GetPlaylist().GetPlaylistId());
				match["results"] = results;
				match["players"] = json::array();

				// populate players
				for (auto& [id, player] : players)
				{
					Player* pl = &player;
					match["players"].push_back(pl->to_json());
				}

				sendMatchRequest(match);

				clearFlags();
			}
		});

	//cvarManager->registerNotifier("goalpost_ping", [this](std::vector<std::string> params) {
	//	ping();
	//	}, "Send PING to the configured endpoint", PERMISSION_ALL);

	LOG("[GOALPOST] GoalPost plugin v{} loaded!", plugin_version);
}

void GoalPost::sendMatchRequest(json& data)
{
	std::string payload = data.dump();
	std::string endpoint = cvarManager->getCvar("goalpost_endpoint").getStringValue();
	
	cpr::Response response = cpr::Post(
		cpr::Url{ endpoint + "/api/matchstats" },
		cpr::Header{ {"Content-Type", "application/json"} },
		cpr::Body{ payload }
	);

	std::string id = data["matchId"];
	if (response.status_code == 200) {
		LOG("Sent match[{}] - {}", id, response.status_code);
	}
	else
	{
		LOG("Failed to send match[{}] - {}", id, response.status_code);
	}
}

std::string GoalPost::getMapName(std::string internalMap)
{
	bool found = false;
	for (const auto& pair : FreeplayMaps) {
		if (pair.first == internalMap) {
			found = true;
			return pair.second;
		}
	}

	if (!found) {
		return "UNK";
	}
}

std::string GoalPost::getPlaylistString(int playlistId)
{
	auto it = PlaylistIdToString.find(playlistId);
	if (it != PlaylistIdToString.end())
	{
		return it->second;
	}
	else
	{
		return "UNK";
	}
}

void GoalPost::clearFlags()
{
	players.clear();
	isOvertime = false;
	isInGame = false;
}

void GoalPost::onUnload()
{

	gameWrapper->UnhookEvent(ON_HANDLE_STAT_TICKER);
	gameWrapper->UnhookEventPost(ON_MATCH_DESTROYED);
	gameWrapper->UnhookEventPost(ON_OVERTIME_UPDATED);
	gameWrapper->UnhookEventPost(ON_COUNTDOWN_BEGINSTATE);
	gameWrapper->UnhookEventPost(ON_MATCH_WINNER_SET);
	LOG("GoalPost Plugin Unloaded");
}
