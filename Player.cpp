#include "pch.h"
#include "Player.h"

json Player::to_json() const
{
    return {
        {"id", id},
        {"name", name},
        {"platform", platform},
        {"teamNum", teamNum},
        {"clear", _clear},
        {"center", _center},
        {"aerialhit", _aerialhit},
        {"firsttouch", _firsttouch},
        {"owngoal", _owngoal},
        {"cartouches", _cartouches},
        {"fastestgoal", _fastestgoal},
        {"slowestgoal", _slowestgoal},
        {"boostpickups", _boostpickups},
        {"furthestgoal", _furthestgoal},
        {"mostballtouches", _mostballtouches},
        {"mostboostpickups", _mostboostpickups},
        {"fewestballtouches", _fewestballtouches},
        {"win", _win},
        {"loss", _loss},
        {"timeplayed", _timeplayed},
        {"bicyclehit", _bicyclehit},
        {"shot", _shot},
        {"assist", _assist},
        {"save", _save},
        {"epicsave", _epicsave},
        {"goal", _goal},
        {"aerialgoal", _aerialgoal},
        {"backwardsgoal", _backwardsgoal},
        {"bicyclegoal", _bicyclegoal},
        {"longgoal", _longgoal},
        {"turtlegoal", _turtlegoal},
        {"overtimegoal", _overtimegoal},
        {"poolshot", _poolshot},
        {"savior", _savior},
        {"lowfive", _lowfive},
        {"hattrick", _hattrick},
        {"highfive", _highfive},
        {"playmaker", _playmaker},
        {"mvp", _mvp},
        {"demos", _demo},
        {"demoed", _killed},
        {"touches", _touches},
        {"mmr", _mmr},
        {"rank", rank}
    };
}

std::string Player::to_string() const
{
	std::ostringstream ss;
    ss << "Player[" << id << " id, " << name << " name, " << platform << " platform, " << teamNum << " teamNum, "
        << _clear << " clear, " << _center << " center, " << _aerialhit << " aerialhit, " << _firsttouch << " firsttouch, "
        << _owngoal << " owngoal, " << _cartouches << " cartouches, " << _fastestgoal << " fastestgoal, " << _slowestgoal << " slowestgoal, "
        << _boostpickups << " boostpickups, " << _furthestgoal << " furthestgoal, " << _mostballtouches << " mostballtouches, "
        << _mostboostpickups << " mostboostpickups, " << _fewestballtouches << " fewestballtouches, " << _win << " win, "
        << _loss << " loss, " << _timeplayed << " timeplayed, " << _bicyclehit << " bicyclehit, " << _shot << " shots, "
        << _assist << " assists, " << _save << " saves, " << _epicsave << " epicsaves, " << _goal << " goals, "
        << _aerialgoal << " aerialgoal, " << _backwardsgoal << " backwardsgoal, " << _bicyclegoal << " bicyclegoal, "
        << _longgoal << " longgoal, " << _turtlegoal << " turtlegoal, " << _overtimegoal << " overtimegoal, "
        << _poolshot << " poolshot, " << _savior << " savior, " << _lowfive << " lowfive, " << _hattrick << " hattrick, "
        << _highfive << " highfive, " << _playmaker << " playmaker, " << _mvp << " mvp, "
        << _demo << " demos, " << _killed << " demoes, " << _touches << " touches, " << _mmr << " mmr, " << rank << " rank]";

	return ss.str();
}

std::string Player::to_scoreboard() const
{
    std::ostringstream ss;
    ss << "Player[" << name << "] goals|" << _goal << "|assists|" << _assist << "|saves|" << (_save + _epicsave) << "|shots|" << _shot;
    return ss.str();
}

void Player::save(int epic) { epic == 0 ? _save++ : _epicsave++; }
void Player::demoed() { _killed++; }
void Player::clear() { ++_clear; }
void Player::center() { ++_center; }
void Player::aerialhit() { ++_aerialhit; }
void Player::firsttouch() { ++_firsttouch; }
void Player::owngoal() { ++_owngoal; }
void Player::cartouches() { ++_cartouches; }
void Player::fastestgoal() { ++_fastestgoal; }
void Player::slowestgoal() { ++_slowestgoal; }
void Player::boostpickups() { ++_boostpickups; }
void Player::furthestgoal() { ++_furthestgoal; }
void Player::mostballtouches() { ++_mostballtouches; }
void Player::mostboostpickups() { ++_mostboostpickups; }
void Player::fewestballtouches() { ++_fewestballtouches; }
void Player::win() { ++_win; }
void Player::loss() { ++_loss; }
void Player::timeplayed() { ++_timeplayed; }
void Player::bicyclehit() { ++_bicyclehit; }
void Player::shot() { ++_shot; }
void Player::assist() { ++_assist; }
void Player::goal() { ++_goal; }
void Player::aerialgoal() { ++_aerialgoal; }
void Player::backwardsgoal() { ++_backwardsgoal; }
void Player::bicyclegoal() { ++_bicyclegoal; }
void Player::longgoal() { ++_longgoal; }
void Player::turtlegoal() { ++_turtlegoal; }
void Player::overtimegoal() { ++_overtimegoal; }
void Player::poolshot() { ++_poolshot; }
void Player::savior() { ++_savior; }
void Player::lowfive() { ++_lowfive; }
void Player::hattrick() { ++_hattrick; }
void Player::highfive() { ++_highfive; }
void Player::playmaker() { ++_playmaker; }
void Player::demo() { ++_demo; }
void Player::mvp() { ++_mvp; }

void Player::setMMR(const int inMMR)
{
	_mmr = inMMR;
}

void Player::setRank(const std::string& inRank)
{
	rank = inRank;
}

std::string Player::getName()
{
	return name;
}

std::string Player::getId()
{
	return id;
}