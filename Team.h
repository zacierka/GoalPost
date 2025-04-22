#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Team {
public:
	std::string name;
	int teamNumber;
	int teamIndex;
	int goals;
	
	void getJson(json &team) {
		team["name"] = name;
		team["goals"] = goals;
	};
};