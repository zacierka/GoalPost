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
	gameWrapper->HookEventPost(ON_MATCH_ENDED,
		[this](std::string eventName) {
			if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
				collectAndSendMatchStats();
			}
		});

	cvarManager->registerNotifier("goalpost_test", [this](std::vector<std::string> params) {
		ping();
		}, "Send PING to the configured endpoint", PERMISSION_ALL);

	LOG("GoalPost plugin loaded!");
}
void GoalPost::collectAndSendMatchStats()
{
	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (!server) {
		LOG("Not in an online game!");
		return;
	}

	// Check if it's a competitive match
	if (!isCompetitiveMatch(server)) {
		LOG("Not a competitive match, skipping stats collection");
		return;
	}

	LOG("Collecting match stats...");


	auto playlist = server.GetPlaylist();
	if (!playlist)
	{
		LOG("Null playlist");
		return;
	}


	// Check if playlist is competitive
	int playlistId = playlist.GetPlaylistId();

	std::string matchId = server.GetMatchGUID();
	bool isWinner = isLocalPlayerWinner(server);
	std::string gamemode = getPlaylistString(playlistId);
	int blueTeamScore = server.GetTeams().Get(0).GetScore();
	int orangeTeamScore = server.GetTeams().Get(1).GetScore();

	PriWrapper localPlayer = server.GetLocalPrimaryPlayer().GetPRI();
	MMRWrapper mw = gameWrapper->GetMMRWrapper();
	float mmr = mw.GetPlayerMMR(localPlayer.GetUniqueIdWrapper(), localPlayer.GetPlatform());

	json player = {
		{ "goals" , localPlayer.GetMatchGoals() },
		{ "uniqueId" , localPlayer.GetUniqueIdWrapper().GetIdString()},
		{ "name" , localPlayer.GetPlayerName().ToString()},
		{ "shots" , localPlayer.GetMatchShots() },
		{ "demos" , localPlayer.GetKills() },
		{ "saves" , localPlayer.GetMatchSaves() },
		{ "assists", localPlayer.GetMatchAssists() },
		{ "touches" , localPlayer.GetBallTouches() },
		{ "teamNum" , localPlayer.GetTeamNum() },
		{ "mmr", mmr },
		{ "platform" , getPlatformString(localPlayer.GetPlatform())}
	};

	const auto now = std::chrono::system_clock::now();
	std::string datetime = std::format("{:%FT%TZ}", now);
	
	// needs MMR, inParty
	json game_data = {
		{ "matchId" , matchId },
		{ "datetime" , datetime },
		{ "map" , gameWrapper->GetCurrentMap() },
		{ "ffTeam" , server.GetbForfeit() },
		{ "gamemode", gamemode },
		{ "blueTeamScore" , blueTeamScore },
		{ "orangeTeamScore" , orangeTeamScore },
		{ "isWinner" , isWinner },
		{ "isOvertime" , isOvertime },
		{ "score" , server.GetTotalScore() },
		{ "localPlayer", player }
	};

	std::string serializedData = game_data.dump();

	sendStats(serializedData);

}

void GoalPost::ping()
{
	LOG("pinging endpoint...");
	std::string url = cvarManager->getCvar("goalpost_endpoint").getStringValue();
	std::thread([url]() {
		try {
			cpr::Response r = cpr::Get(cpr::Url{ url });

			if (r.status_code == 201)
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

bool GoalPost::isFF(ServerWrapper server)
{
	bool isWinner = isLocalPlayerWinner(server);

	long wasForfeit = server.GetbForfeit(); // true if match ended by forfeit

	
	return false;
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

void GoalPost::onOvertime()
{
	LOG("GAME IS In Overtime");
	isOvertime = true;
}

void GoalPost::resetFlags()
{
	isOvertime = false;
}

void GoalPost::sendStats(std::string& data)
{
	LOG("Sending stats");
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

bool GoalPost::isCompetitiveMatch(ServerWrapper server)
{
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	// Check if playlist is competitive
	int playlistId = playlist.GetPlaylistId();

	LOG("Game mode ID is {}", playlistId);

	return (playlistId == 10 || // 1v1 Ranked
		playlistId == 11 || // 2v2 Ranked
		playlistId == 13 // 3v3 Standard
		);
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

void GoalPost::onUnload()
{

	gameWrapper->UnhookEvent(ON_ALL_TEAMS_CREATED);
	gameWrapper->UnhookEvent(ON_MATCH_ENDED);
	gameWrapper->UnhookEvent(ON_OVERTIME_UPDATED);

	LOG("GoalPost Plugin Unloaded");
}
