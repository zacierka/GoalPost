#pragma once
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Game
{
public:
	int teamSize = 0;
	int SecondsRemaining = 0;
	int SecondsElapsed = 0;
	int playlist_type = 5;
	
	void getJson(json &session) {
		session["playlist_name"] = this->GetPlaylistName();
		session["playlist_id"] = playlist_type;
		session["team_size"] = teamSize;
		session["seconds_remaining"] = SecondsRemaining;
		session["seconds_elapsed"] = SecondsElapsed;
	}

	std::string GetPlaylistName() {
		switch (this->playlist_type) {
		case(1):
			return "Casual Duel";
		case(2):
			return "Casual Doubles";
		case(3):
			return "Casual Standard";
		case(4):
			return "Casual Chaos";
		case(5):
			return "Menu";
		case(6):
			return "Private " + std::to_string(this->teamSize) + "v" + std::to_string(this->teamSize);
		case(9):
			return "Freeplay";
		case(10):
			return "Ranked Duel";
		case(11):
			return "Ranked Doubles";
		case(12):
			return "Ranked Solo Standard";
		case(13):
			return "Ranked Standard";
		case(14):
			return "Mutator Mashup";
		case(22):
			return "Tournament " + std::to_string(this->teamSize) + "v" + std::to_string(this->teamSize);
		case(27):
			return "Ranked Hoops";
		case(28):
			return "Ranked Rumble";
		case(29):
			return "Ranked Dropshot";
		case(30):
			return "Ranked Snowday";
		default:
			return "Charting unknown territories";
		}
	}
private:

};