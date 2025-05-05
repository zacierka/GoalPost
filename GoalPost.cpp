#include "pch.h"
#include "GoalPost.h"
#include <thread>
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameEvent/TutorialWrapper.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/http/HttpWrapper.h"
#include "bakkesmod/wrappers/cvarmanagerwrapper.h"
#include <chrono>
// #define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

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
	cvarManager->registerCvar("goalpost_endpoint", "localhost", "Endpoint URL for match stats");
	cvarManager->registerCvar("goalpost_enabled", "1", "Enable/disable match stats collection", true, true, 0, true, 1);

	gameWrapper->HookEventPost(ON_ALL_TEAMS_CREATED,
		[this](std::string eventName) {
			LOG("@@@@@ Match started @@@@@");
			resetFlags();
		});

	gameWrapper->HookEventPost(ON_OVERTIME_UPDATED,
		[this](std::string eventName) {
			onOvertime();
		});
	gameWrapper->HookEventPost(ON_MATCH_ENDED,
		[this](std::string eventName) {
			if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
				LOG("@@@@@ Match ended - Match stats collection enabled @@@@@");
				collectAndSendMatchStats();
			}
			else {
				LOG("@@@@@ Match ended - Match stats collection disabled @@@@@");
			}
		});

	cvarManager->registerNotifier("goalpost_test", [this](std::vector<std::string> params) {
		testStats();
		}, "Send test match stats to the configured endpoint", PERMISSION_ALL);

	LOG("Goal Websocket plugin loaded!");
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

	// for each player list

	sendStats(game_data.dump());

}

void GoalPost::testStats()
{
	json player = {
		{ "goals" , 5 },
		{ "uniqueId" , "Steam|76561198040749114|0"},
		{ "name" , "Mikey" },
		{ "shots" , 3 },
		{ "demos" , 5 },
		{ "saves" , 1 },
		{ "assists", 12 },
		{ "touches" ,35 },
		{ "teamNum" ,0 },
		{ "platform" , "Steam"}
	};

	const auto now = std::chrono::system_clock::now();
	std::string datetime = std::format("{:%FT%TZ}", now);

	// needs MMR, inParty
	json game_data = {
		{ "matchId" , "159ADDCE11F029512CC831802D90E774" },
		{ "datetime" , datetime },
		{ "map" , "woods_p"},
		{ "ffTeam" , 1 },
		{ "gamemode", "Standard (Ranked)"},
		{ "blueTeamScore" , 0 },
		{ "orangeTeamScore" , 1 },
		{ "isWinner" , true },
		{ "isOvertime" , false },
		{ "score" , 1},
		{ "localPlayer", player }
	};

	// for each player list

	sendStats(game_data.dump());
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

void GoalPost::sendStats(std::string data)
{
	LOG("Sending stats");
	std::thread([=]() {
		std::string url = "";
		try {
			url = cvarManager->getCvar("goalpost_endpoint").getStringValue();
			httplib::Client cli(url, 3000);

			auto res = cli.Post("/api/matchstats", data, "application/json");

			if (res && res->status == 200) {
				LOG("Sent {} - Success", res->status);
			}
			else {
				LOG("Sent {} - Unreachable", res->status);
			}
		}
		catch (...)
		{
			LOG("Error... could not send to endpoint {}", url);
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

	LOG("Plugin Unloaded");
}
