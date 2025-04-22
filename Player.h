#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Player {
public:
	std::string playerName;
	int goals;
	int teamNum;
	int assists;
	int saves;
	int playerID;
	int teamNum;
	unsigned long long playerUniqueID;
	int demos;
	int ballTouches;
	float currentBoostAmount;
	float MMR;
	bool isBot;

	void getJson(json &player) {
		player["player_name"] = playerName;
		player["goals"] = goals;
		player["MMR"] = MMR;
		player["assists"] = assists;
		player["saves"] = saves;
		player["player_id"] = playerID;
		player["player_unique_id"] = playerUniqueID;
		player["demos"] = demos;
		player["ball_touches"] = ballTouches;
		player["current_boost_amount"] = currentBoostAmount;
		player["team_number"] = teamNum;
		player["bot"] = isBot;
	};
};