#pragma once
#include <memory>
class CheatFunction;
extern std::shared_ptr<CheatFunction> UpdatePlayers;
extern std::shared_ptr<CheatFunction> UpdateBosses;
extern std::shared_ptr<CheatFunction> UpdateGrunts;
extern void DrawPlayersEsp();
extern void DrawBossesEsp();
extern bool IsValidHP(int hp);
