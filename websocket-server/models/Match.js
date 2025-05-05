const mongoose = require('mongoose');

const localPlayerSchema = new mongoose.Schema({
  assists: Number,
  demos: Number,
  goals: Number,
  name: String,
  platform: String,
  saves: Number,
  shots: Number,
  teamNum: Number,
  touches: Number,
  uniqueId: String
});

const matchSchema = new mongoose.Schema({
  blueTeamScore: Number,
  datetime: Date,
  ffTeam: Number, // 1=FF
  gamemode: String,
  isOvertime: Boolean,
  isWinner: Boolean,
  localPlayer: localPlayerSchema,
  map: String,
  matchId: String,
  orangeTeamScore: Number,
  score: Number
});

module.exports = mongoose.model('Match', matchSchema);
