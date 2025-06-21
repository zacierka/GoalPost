#pragma once

#include "pch.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Player {
	// id, name, goals, shots, saves, assists, demos, touches, teamNum, mmr, platform
private:
	std::string id;
	std::string name;
	std::string platform;
	std::string rank;

	int teamNum;
	int _demo;
	int _killed;
	int _touches;
	int _mmr;
	int _clear;
	int _center;
	int _aerialhit;
	int _firsttouch;
	int _owngoal;
	int _cartouches;
	int _fastestgoal;
	int _slowestgoal;
	int _boostpickups;
	int _furthestgoal;
	int _mostballtouches;
	int _mostboostpickups;
	int _fewestballtouches;
	int _win;
	int _loss;
	int _timeplayed;
	int _bicyclehit;
	int _shot;
	int _assist;
	int _save;
	int _epicsave;
	int _goal;
	int _aerialgoal;
	int _backwardsgoal;
	int _bicyclegoal;
	int _longgoal;
	int _turtlegoal;
	int _overtimegoal;
	int _poolshot;
	int _savior;
	int _lowfive;
	int _hattrick;
	int _highfive;
	int _playmaker;
	int _mvp;

public:
	Player(const std::string& id, const std::string& playerName, const int teamNum) : 
		id(id), name(playerName), platform("steam"), teamNum(0),
		_clear(0), _center(0), _aerialhit(0), _firsttouch(0), _owngoal(0),
		_cartouches(0), _fastestgoal(0), _slowestgoal(0), _boostpickups(0), _furthestgoal(0),
		_mostballtouches(0), _mostboostpickups(0), _fewestballtouches(0), _win(0), _loss(0),
		_timeplayed(0), _bicyclehit(0), _shot(0), _assist(0), _save(0),
		_epicsave(0), _goal(0), _aerialgoal(0), _backwardsgoal(0), _bicyclegoal(0),
		_longgoal(0), _turtlegoal(0), _overtimegoal(0), _poolshot(0), _savior(0),
		_lowfive(0), _hattrick(0), _highfive(0), _playmaker(0), _demo(0),
		_mvp(0), _killed(0), _mmr(100), _touches(0) { }

	void demo(); // demo
	void demoed(); // demoed
	// touches added at end of game
	void setMMR(const int mmr); // MMR
	void setRank(const std::string& rank);
	// store advanced stats. map or vector
	std::string getName();
	std::string getId();

	json to_json() const;
	std::string to_string() const;
	std::string to_scoreboard() const;

	void clear();
	void center();
	void aerialhit();
	void firsttouch();
	void owngoal();
	void cartouches();

	void fastestgoal();
	void slowestgoal();
	void boostpickups();
	void furthestgoal();

	void mostballtouches();
	void mostboostpickups();
	void fewestballtouches();

	void win();
	void loss();
	void timeplayed();

	void bicyclehit();
	void shot();
	void assist();
	void save(int epic = 0); // epic = 1 for EpicSave

	void goal();
	void aerialgoal();
	void backwardsgoal();
	void bicyclegoal();
	void longgoal();
	void turtlegoal();
	void overtimegoal();
	void poolshot();

	void savior();
	void lowfive();
	void hattrick();
	void highfive();
	void playmaker();
	void mvp();

};