#include "pch.h"
#include "GoalPost.h"
#include <unordered_map>
#include <functional>

const int COMPETITIVE_1V1 = 10;
const int COMPETITIVE_2V2 = 11;
const int COMPETITIVE_3V3 = 13;


const std::unordered_map<std::string, std::function<void(Player*)>> eventHandlers = {
    {"Clear",       [](Player* p) { p->clear(); }},
    {"Center",      [](Player* p) { p->center(); }},
    {"AerialHit",   [](Player* p) { p->aerialhit(); }},
    {"FirstTouch",  [](Player* p) { p->firsttouch(); }},
    {"OwnGoal",     [](Player* p) { p->owngoal(); }},
    {"CarTouches",  [](Player* p) { p->cartouches(); }},
    {"FastestGoal", [](Player* p) { p->fastestgoal(); }},
    {"SlowestGoal", [](Player* p) { p->slowestgoal(); }},
    {"BoostPickups",[](Player* p) { p->boostpickups(); }},
    {"FurthestGoal",[](Player* p) { p->furthestgoal(); }},
    {"MostBallTouches", [](Player* p) { p->mostballtouches(); }},
    {"MostBoostPickups", [](Player* p) { p->mostboostpickups(); }},
    {"FewestBallTouches", [](Player* p) { p->fewestballtouches(); }},
    {"Win",        [](Player* p) { p->win(); }},
    {"Loss",       [](Player* p) { p->loss(); }},
    {"TimePlayed", [](Player* p) { p->timeplayed(); }},
    {"BicycleHit", [](Player* p) { p->bicyclehit(); }},
    {"Shot",       [](Player* p) { p->shot(); }},
    {"Assist",     [](Player* p) { p->assist(); }},
    {"Save",       [](Player* p) { p->save(0); }},
    {"EpicSave",   [](Player* p) { p->save(1); }},
    {"Goal",       [](Player* p) { p->goal(); }},
    {"AerialGoal", [](Player* p) { p->aerialgoal(); }},
    {"BackwardsGoal", [](Player* p) { p->backwardsgoal(); }},
    {"BicycleGoal", [](Player* p) { p->bicyclegoal(); }},
    {"LongGoal",   [](Player* p) { p->longgoal(); }},
    {"TurtleGoal", [](Player* p) { p->turtlegoal(); }},
    {"OvertimeGoal", [](Player* p) { p->overtimegoal(); }},
    {"PoolShot",   [](Player* p) { p->poolshot(); }},
    {"Savior",     [](Player* p) { p->savior(); }},
    {"LowFive",    [](Player* p) { p->lowfive(); }},
    {"HatTrick",   [](Player* p) { p->hattrick(); }},
    {"HighFive",   [](Player* p) { p->highfive(); }},
    {"Playmaker",  [](Player* p) { p->playmaker(); }},
    {"Demolish",   [](Player* p) { p->demo(); }},
    {"MVP",        [](Player* p) { p->mvp(); }}
};


// match: id, datetime, map, gamemode, results[], players[]
//json match;
// results: score, blue, orange, ot, ff, winner
//json results;
// player: id, name, goals, shots, saves, assists, demos, touches, teamNum, mmr, platform
//json player;
bool GoalPost::isLocalPlayer(PriWrapper pri)
{
	if (pri.IsNull())
		return (false);

	ServerWrapper server = (isInGame ? gameWrapper->GetOnlineGame() : gameWrapper->GetGameEventAsServer());
	if (server.IsNull())
		return (false);

	PlayerControllerWrapper player = server.GetLocalPrimaryPlayer();
	if (player.IsNull())
		return (false);

	PriWrapper playerPRI = player.GetPRI();
	if (playerPRI.IsNull())
		return (false);

	return (pri.GetUniqueIdWrapper().GetUID() == playerPRI.GetUniqueIdWrapper().GetUID());
}

bool GoalPost::isCompetitiveGame()
{
	if (!gameWrapper->IsInOnlineGame()) return false;

	ServerWrapper server = gameWrapper->GetOnlineGame();
	if (!server) return false;

	auto playlist = server.GetPlaylist();
	if (!playlist) return false;

	int playlistId = playlist.GetPlaylistId();

	return (playlistId == COMPETITIVE_1V1 || // 1v1 Ranked
		playlistId == COMPETITIVE_2V2 || // 2v2 Ranked
		playlistId == COMPETITIVE_3V3 // 3v3 Standard
		);
}

// called any time a stat should appear in the center of the screen
// catches all stats, but only for the primary player
// Demolish is a death, Demolition is an extermination
// KO_ knockout stats
//void GoalPost::onStatEvent(ServerWrapper caller, void* args)
//{
//	StatEventParams* pstats = (StatEventParams*) args;
//	PriWrapper playerPRI = PriWrapper(pstats->PRI);
//	StatEventWrapper event = StatEventWrapper(pstats->StatEvent);
//
//	std::string name = event.GetEventName();
//	LOG("Recieved event {}", name);
//}

// called any time a stat should appear on the top right ticker
// all players? 
void GoalPost::onStatTickerEvent(ServerWrapper caller, void* args)
{
	// only want online games (& competitive?)
	if (!gameWrapper->IsInOnlineGame()) return;

	StatTickerParams* pstats = (StatTickerParams*) args;

	CarWrapper me = gameWrapper->GetLocalCar();
	PriWrapper receiver = PriWrapper(pstats->Receiver);
	PriWrapper victim = PriWrapper(pstats->Victim);
	StatEventWrapper event = StatEventWrapper(pstats->StatEvent);

	std::string name = event.GetEventName();

	bool iam_receiver = (!receiver.IsNull() && isLocalPlayer(receiver));
	bool iam_victim = (!victim.IsNull() && isLocalPlayer(victim));
	bool team_receiver = (!me.IsNull() && !receiver.IsNull() && me.GetTeamNum2() == receiver.GetTeamNum2());
	bool team_victim = (!me.IsNull() && !victim.IsNull() && me.GetTeamNum2() == victim.GetTeamNum2());

    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    if (it != players.end())
    {
        Player* p = &it->second;

        auto itEvent = eventHandlers.find(name);
        if (itEvent != eventHandlers.end())
        {
            itEvent->second(p);
        }
        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    }

    //if (name == "Clear")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Center")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "AerialHit")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "FirstTouch")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "OwnGoal")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "CarTouches")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "FastestGoal")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "SlowestGoal")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "BoostPickups")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "FurthestGoal")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "MostBallTouches")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "MostBoostPickups")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "FewestBallTouches")
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Win") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Loss") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "TimePlayed") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "BicycleHit") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Shot") {
    //    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    //    if (it != players.end()) {
    //        Player* p = &it->second;
    //        p->shot();
    //        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //    }
    //}
    //else if (name == "Assist") {
    //    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    //    if (it != players.end()) {
    //        Player* p = &it->second;
    //        p->assist();
    //        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //    }
    //}
    //else if (name == "Save") {
    //    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    //    if (it != players.end()) {
    //        Player* p = &it->second;
    //        p->save();
    //        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //    }
    //}
    //else if (name == "EpicSave") {
    //    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    //    if (it != players.end()) {
    //        Player* p = &it->second;
    //        p->save(1);
    //        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //    }
    //}
    //else if (name == "Goal") { // simple goal. other goal types called below
    //    auto it = players.find(receiver.GetUniqueIdWrapper().GetIdString());
    //    if (it != players.end()) {
    //        Player* p = &it->second;
    //        p->goal();
    //        gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //    }
    //}
    //else if (name == "AerialGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "BackwardsGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "BicycleGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "LongGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "TurtleGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "OvertimeGoal") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "PoolShot") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "FirstTouch") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Savior") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "LowFive") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "HatTrick") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "HighFive") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Playmaker") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "Demolish") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else if (name == "MVP") {
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " |--> " + name);
    //}
    //else
    //{
    //    gameWrapper->LogToChatbox(receiver.GetPlayerName().ToString() + " UNK onStatEvent " + name + "[" + std::to_string(event.GetPoints()) + "]");
    //}
}