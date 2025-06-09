#include "pch.h"
#include "GoalPost.h"
#include <thread>
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/http/HttpWrapper.h"
#include "bakkesmod/utilities/LoadoutUtilities.h" // used to get the car data
#include "bakkesmod/wrappers/cvarmanagerwrapper.h"
#include <chrono>
#include <cpr/cpr.h>

#define ON_ALL_TEAMS_CREATED "Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated"
#define ON_OVERTIME_UPDATED "Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated"
#define ON_MATCH_ENDED "Function TAGame.GameEvent_Soccar_TA.OnMatchEnded"
#define ON_MATCH_WINNER_SET "Function TAGame.GameEvent_Soccar_TA.OnMatchWinnerSet"
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

	gameWrapper->HookEventPost(ON_ALL_TEAMS_CREATED,
		[this](std::string eventName) {
			resetFlags();
		});

	gameWrapper->HookEventPost(ON_OVERTIME_UPDATED,
		[this](std::string eventName) {
			onOvertime();
		});
	gameWrapper->HookEventPost(ON_MATCH_WINNER_SET,
		[this](std::string eventName) {
			if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
				collectAndSendMatchStats();
			}
		});

	cvarManager->registerNotifier("goalpost_ping", [this](std::vector<std::string> params) {
		ping();
		}, "Send PING to the configured endpoint", PERMISSION_ALL);

	LOG("[GOALPOST] GoalPost plugin v{} loaded!", plugin_version);
}


void GoalPost::collectAndSendMatchStats()
{
	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (!server) {
		LOG("[GOALPOST] Not in an online game!");
		return;
	}

	// Check if it's a competitive match
	if (!isCompetitiveMatch(server)) {
		LOG("[GOALPOST] Not a competitive match, skipping stats collection");
		return;
	}
	LOG("[GOALPOST] Collecting match stats...");

	auto playlist = server.GetPlaylist();
	if (!playlist)
	{
		LOG("[GOALPOST] Null playlist");
		return;
	}

	PriWrapper localPlayer = server.GetLocalPrimaryPlayer().GetPRI();
	MMRWrapper mw = gameWrapper->GetMMRWrapper();

	int playlistId = playlist.GetPlaylistId();
	float mmr = mw.GetPlayerMMR(localPlayer.GetUniqueIdWrapper(), playlistId);
	

	const auto now = std::chrono::system_clock::now();
	std::string datetime = std::format("{:%FT%TZ}", now);

	json results;
	results["score"] = server.GetTotalScore();
	results["bScore"] = server.GetTeams().Get(0).GetScore();
	results["oScore"] = server.GetTeams().Get(1).GetScore();
	results["isOT"] = isOvertime;
	results["isFF"] = (server.GetbForfeit() == 1);
	results["isWinner"] = isLocalPlayerWinner(server);

	json match;
	match["matchId"] = server.GetMatchGUID();
	match["datetime"] = datetime;
	match["map"] = getMapName(gameWrapper->GetCurrentMap());
	match["gamemode"] = getPlaylistString(playlistId);
	match["results"] = results;

	json players = json::array();
	getPlayersStats(server.GetPRIs(), players, playlistId);
	match["players"] = players;

	std::string serializedData = match.dump();

	sendStats(serializedData);
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

void GoalPost::ping()
{
	LOG("[GOALPOST] pinging endpoint...");
	std::string url = cvarManager->getCvar("goalpost_endpoint").getStringValue();
	std::thread([url]() {
		try {
			cpr::Response r = cpr::Get(cpr::Url{ url });

			if (r.status_code == 200)
			{
				LOG("[SUCCESS] Response code: {}", r.status_code);
			}
			else
			{
				LOG("[FAILED] Cannot find endpoint: {}", url);
			}
		}
		catch (const std::exception& e)
		{
			LOG("[ERROR] Could not send ping {}", e.what());
		}

		}).detach();
}

void GoalPost::onOvertime()
{
	LOG("[GOALPOST] GAME IS In Overtime");
	isOvertime = true;
}

int getPlayerMMR(UniqueIDWrapper uid, int playlist) {
	MMRWrapper mw = _globalGameWrapper->GetMMRWrapper();
	return mw.GetPlayerMMR(uid, playlist);
}

std::string GoalPost::getPlaylistString(int playlistId)
{
	std::string gamemode;
	switch (playlistId)
	{
	case 10:
		gamemode = "Duel (Ranked)";
		break;
	case 11:
		gamemode = "Doubles (Ranked)";
		break;
	case 13:
		gamemode = "Standard (Ranked)";
		break;
	default:
		gamemode = "Standard";
		break;
	}
	return gamemode;
}

int getMMR(UniqueIDWrapper uid, int playlist) {
	if (!_globalGameWrapper) return 0;

	MMRWrapper mw = _globalGameWrapper->GetMMRWrapper();
	return mw.GetPlayerMMR(uid, playlist);
}

bool GoalPost::isCompetitiveMatch(ServerWrapper server)
{
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;


	// Check if playlist is competitive
	int playlistId = playlist.GetPlaylistId();

	return (playlistId == 10 || // 1v1 Ranked
		playlistId == 11 || // 2v2 Ranked
		playlistId == 13 // 3v3 Standard
		);
}

bool GoalPost::isLocalPlayerWinner(ServerWrapper server)
{

	PriWrapper localPlayer = server.GetLocalPrimaryPlayer().GetPRI();
	ArrayWrapper<TeamWrapper> teams = server.GetTeams();

	int myScore = teams.Get(localPlayer.GetTeamNum()).GetScore();
	int otherScore;

	if (localPlayer.GetTeamNum() == 0) {
		otherScore = teams.Get(1).GetScore();
	}
	else {
		otherScore = teams.Get(0).GetScore();
	}

	return myScore > otherScore;
}

std::string GoalPost::getPlatformString(OnlinePlatform plat)
{
	std::string platform;

	switch (plat)
	{
	case OnlinePlatform_Unknown:  platform = "Unknown"; break;
	case OnlinePlatform_Steam:    platform = "Steam"; break;
	case OnlinePlatform_PS4:      platform = "PS4"; break;
	case OnlinePlatform_PS3:      platform = "PS3"; break;
	case OnlinePlatform_Dingo:    platform = "Xbox"; break;
	case OnlinePlatform_QQ:       platform = "QQ"; break;
	case OnlinePlatform_OldNNX:   platform = "OldNNX"; break;
	case OnlinePlatform_NNX:      platform = "NNX"; break;
	case OnlinePlatform_PsyNet:   platform = "PsyNet"; break;
	case OnlinePlatform_Deleted:  platform = "Deleted"; break;
	case OnlinePlatform_WeGame:   platform = "WeGame"; break;
	case OnlinePlatform_Epic:     platform = "Epic"; break;
	case OnlinePlatform_MAX:      platform = "MAX"; break;
	default:                      platform = "Invalid"; break;
	}

	return platform;
}

void GoalPost::getPlayersStats(ArrayWrapper<PriWrapper> pris, json& players, int playlistId)
{
	for (auto player : pris) {
		LOG("[GOALPOST] Parsing player: {}", player.GetPlayerName().ToString());

		json p;
		p["uniqueId"] = player.GetUniqueIdWrapper().GetIdString();
		p["name"] = player.GetPlayerName().ToString();
		p["goals"] = player.GetMatchGoals();
		p["shots"] = player.GetMatchShots();
		p["saves"] = player.GetMatchSaves();
		p["assists"] = player.GetMatchAssists();
		p["demos"] = player.GetKills();
		p["touches"] = player.GetBallTouches();
		p["teamNum"] = player.GetTeamNum();
		p["mmr"] = getMMR(player.GetUniqueIdWrapper(), playlistId);
		p["platform"] = getPlatformString(player.GetPlatform());
		// left game stat
		players.push_back(p);
	}
}

void GoalPost::resetFlags()
{
	isOvertime = false;
}

void GoalPost::sendStats(std::string& data)
{
	LOG("[GOALPOST] Sending stats");
	std::string url = cvarManager->getCvar("goalpost_endpoint").getStringValue();
	std::thread([url, data]() {
		try {
			
			cpr::Response r = cpr::Post(
				cpr::Url{ url + "/api/matchstats" },
				cpr::Header{ {"Content-Type", "application/json"} },
				cpr::Body{ data }
			);

			if (r.status_code == 201)
			{
				LOG("[SUCCESS] Response code: {}", r.status_code);
			}
			else
			{
				LOG("[FAILED] Cannot find endpoint: {}", url);
			}
		}
		catch (...)
		{
			LOG("[ERROR] Could not send to endpoint {}", url);
		}

		}).detach();
}

void GoalPost::onUnload()
{

	gameWrapper->UnhookEvent(ON_ALL_TEAMS_CREATED);
	gameWrapper->UnhookEvent(ON_MATCH_ENDED);
	gameWrapper->UnhookEvent(ON_OVERTIME_UPDATED);

	LOG("GoalPost Plugin Unloaded");
}
