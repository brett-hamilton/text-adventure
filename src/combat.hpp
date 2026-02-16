#pragma once

#include "types.hpp"

enum class CombatResult { Won, Fled, Died };

CombatResult run_combat(Player& player, Enemy& enemy,
                        int bonus_atk, int bonus_def,
                        const std::map<std::string, Item>& items);
