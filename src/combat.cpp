#include "combat.hpp"
#include <algorithm>
#include <iostream>
#include <print>
#include <random>
#include <string>

static std::mt19937& rng() {
    static std::mt19937 gen(std::random_device{}());
    return gen;
}

static std::string find_potion_in_inventory(const Player& player,
                                             const std::map<std::string, Item>& items) {
    for (auto& id : player.inventory) {
        auto it = items.find(id);
        if (it != items.end() && it->second.type == ItemType::Potion) {
            return id;
        }
    }
    return "";
}

CombatResult run_combat(Player& player, Enemy& enemy,
                        int bonus_atk, int bonus_def,
                        const std::map<std::string, Item>& items) {
    std::println("\n=== COMBAT: {} ===", enemy.name);
    std::println("{} — HP: {}/{}, ATK: {}, DEF: {}",
                 enemy.name, enemy.hp, enemy.max_hp, enemy.attack, enemy.defense);
    std::println("You — HP: {}/{}, ATK: {}+{}, DEF: {}+{}\n",
                 player.hp, player.max_hp, player.attack, bonus_atk, player.defense, bonus_def);

    while (enemy.hp > 0 && player.hp > 0) {
        std::print("[A]ttack / [U]se potion / [F]lee > ");
        std::string choice;
        if (!std::getline(std::cin, choice) || choice.empty()) {
            continue;
        }

        char c = std::tolower(static_cast<unsigned char>(choice[0]));

        if (c == 'a') {
            int dmg = std::max(1, (player.attack + bonus_atk) - enemy.defense);
            enemy.hp = std::max(0, enemy.hp - dmg);
            std::println("You strike the {} for {} damage! (Enemy HP: {})",
                         enemy.name, dmg, enemy.hp);
        } else if (c == 'u') {
            auto potion_id = find_potion_in_inventory(player, items);
            if (potion_id.empty()) {
                std::println("You don't have any potions!");
                continue;
            }
            auto& potion = items.at(potion_id);
            player.hp = std::min(player.max_hp, player.hp + potion.value);
            std::erase(player.inventory, potion_id);
            std::println("You drink the {}! HP restored to {}.", potion.name, player.hp);
        } else if (c == 'f') {
            std::uniform_int_distribution<int> dist(0, 1);
            if (dist(rng())) {
                std::println("You flee from the {}!", enemy.name);
                return CombatResult::Fled;
            } else {
                std::println("You failed to escape!");
            }
        } else {
            std::println("Choose [A]ttack, [U]se potion, or [F]lee.");
            continue;
        }

        // Enemy retaliates if still alive
        if (enemy.hp > 0) {
            int dmg = std::max(1, enemy.attack - (player.defense + bonus_def));
            player.hp = std::max(0, player.hp - dmg);
            std::println("The {} strikes you for {} damage! (Your HP: {})",
                         enemy.name, dmg, player.hp);
        }
    }

    if (player.hp <= 0) {
        std::println("\nYou have been slain by the {}...", enemy.name);
        return CombatResult::Died;
    }

    std::println("\nYou defeated the {}!", enemy.name);
    return CombatResult::Won;
}
