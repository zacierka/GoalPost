#include "pch.h"
#include "GoalPost.h"
#include <cpr/cpr.h>

#define ON_ALL_TEAMS_CREATED "Function TAGame.GameEvent_Soccar_TA.OnAllTeamsCreated"
#define ON_POST_BEGIN_PLAY "Function TAGame.Team_TA.PostBeginPlay"
#define ON_OVERTIME_UPDATED "Function TAGame.GameEvent_Soccar_TA.OnOvertimeUpdated"
#define ON_MATCH_ENDED "Function TAGame.GameEvent_Soccar_TA.OnMatchEnded"
#define ON_MATCH_DESTROYED "Function TAGame.GameEvent_TA.Destroyed"
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
				//if (!gameWrapper->IsInOnlineGame()) return;
				LOG("@@@@@@GameStarted@@@@@@");
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
	gameWrapper->HookEventWithCallerPost<ServerWrapper>("Function TAGame.GFxHUD_TA.HandleStatTickerMessage", std::bind(&GoalPost::onStatTickerEvent, this, std::placeholders::_1, std::placeholders::_2));


	// Game Destroyed Event -> leave early or disconnect
	gameWrapper->HookEventPost(ON_MATCH_DESTROYED,
	[this](std::string eventName) {
		if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
			isInGame = false;
		}
		});

	// Chat Event -> count and store chats.

	gameWrapper->HookEventPost(ON_MATCH_WINNER_SET,
		[this](std::string eventName) {
			if (cvarManager->getCvar("goalpost_enabled").getBoolValue()) {
				/*for (auto& [id, player] : players)
				{
					Player* p = &player;
					LOG(p->to_scoreboard());
				}*/
				players.clear();
				isInGame = false;
			}
		});

	//cvarManager->registerNotifier("goalpost_ping", [this](std::vector<std::string> params) {
	//	ping();
	//	}, "Send PING to the configured endpoint", PERMISSION_ALL);

	LOG("[GOALPOST] GoalPost plugin v{} loaded!", plugin_version);
}

void GoalPost::onUnload()
{

	gameWrapper->UnhookEvent(ON_ALL_TEAMS_CREATED);
	gameWrapper->UnhookEvent(ON_OVERTIME_UPDATED);

	LOG("GoalPost Plugin Unloaded");
}
