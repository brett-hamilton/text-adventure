#include "game.hpp"
#include "combat.hpp"
#include "data_loader.hpp"
#include "parser.hpp"
#include <algorithm>
#include <iostream>
#include <print>

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

Game::Game() {
    auto data = load_game_data("data");
    items_ = std::move(data.items);
    rooms_ = std::move(data.rooms);
    current_room_id_ = data.start_room;

    player_.name = "Adventurer";
    player_.hp = 100;
    player_.max_hp = 100;
    player_.attack = 10;
    player_.defense = 5;
}

void Game::run() {
    std::println("========================================");
    std::println("  THE DUNGEON OF THE SHADOW LORD");
    std::println("========================================\n");
    std::println("You are {}, a brave adventurer who has entered", player_.name);
    std::println("the dungeon to defeat the Shadow Lord.\n");
    std::println("Type 'help' for a list of commands.\n");

    describe_room();

    while (running_) {
        std::print("\n> ");
        std::string input;
        if (!std::getline(std::cin, input)) {
            // EOF (Ctrl-D)
            std::println("\nFarewell, adventurer!");
            break;
        }

        auto cmd = parse_command(input);

        switch (cmd.type) {
        case CommandType::Go:        do_go(cmd.argument); break;
        case CommandType::Look:      do_look(); break;
        case CommandType::Take:      do_take(cmd.argument); break;
        case CommandType::Use:       do_use(cmd.argument); break;
        case CommandType::Equip:     do_equip(cmd.argument); break;
        case CommandType::Attack:    do_attack(); break;
        case CommandType::Inventory: do_inventory(); break;
        case CommandType::Help:      do_help(); break;
        case CommandType::Quit:
            std::println("Farewell, adventurer!");
            running_ = false;
            break;
        case CommandType::Unknown:
            if (!input.empty() && input.find_first_not_of(" \t\r\n") != std::string::npos) {
                std::println("I don't understand that. Type 'help' for commands.");
            }
            break;
        }
    }
}

void Game::describe_room() {
    auto& room = rooms_.at(current_room_id_);
    std::println("--- {} ---", room.name);
    std::println("{}", room.description);

    if (room.enemy) {
        std::println("\nA {} blocks your path! (HP: {}/{})",
                     room.enemy->name, room.enemy->hp, room.enemy->max_hp);
    }

    if (!room.item_ids.empty()) {
        std::print("\nYou see: ");
        for (size_t i = 0; i < room.item_ids.size(); ++i) {
            if (i > 0) std::print(", ");
            auto it = items_.find(room.item_ids[i]);
            if (it != items_.end()) {
                std::print("{}", it->second.name);
            }
        }
        std::println("");
    }

    std::print("Exits: ");
    bool first = true;
    for (auto& [dir, _] : room.exits) {
        if (!first) std::print(", ");
        std::print("{}", dir);
        first = false;
    }
    std::println("");
}

void Game::do_go(const std::string& direction) {
    if (direction.empty()) {
        std::println("Go where? Try: go north, go south, go east, go west");
        return;
    }

    auto& room = rooms_.at(current_room_id_);

    // Can't leave if enemy is present
    if (room.enemy) {
        std::println("The {} blocks your path! You must fight or flee!", room.enemy->name);
        return;
    }

    auto it = room.exits.find(direction);
    if (it == room.exits.end()) {
        std::print("You can't go that way. Exits: ");
        bool first = true;
        for (auto& [dir, _] : room.exits) {
            if (!first) std::print(", ");
            std::print("{}", dir);
            first = false;
        }
        std::println("");
        return;
    }

    auto target = it->second;

    // Vault lock check
    if (target == "shadow_vault") {
        bool has_key = std::find(player_.inventory.begin(), player_.inventory.end(),
                                  "vault_key") != player_.inventory.end();
        if (!has_key) {
            std::println("The iron door is locked. You need a key to enter.");
            return;
        }
        std::println("You use the Vault Key to unlock the iron door...");
        std::erase(player_.inventory, "vault_key");
    }

    current_room_id_ = target;
    describe_room();
}

void Game::do_look() {
    describe_room();
}

std::string Game::find_item_by_name(const std::vector<std::string>& ids,
                                     const std::string& name) const {
    std::string lower_name = to_lower(name);
    for (auto& id : ids) {
        auto it = items_.find(id);
        if (it != items_.end()) {
            std::string lower_item = to_lower(it->second.name);
            if (lower_item.find(lower_name) != std::string::npos) {
                return id;
            }
        }
    }
    return "";
}

void Game::do_take(const std::string& arg) {
    if (arg.empty()) {
        std::println("What do you want to take?");
        return;
    }

    auto& room = rooms_.at(current_room_id_);
    auto item_id = find_item_by_name(room.item_ids, arg);
    if (item_id.empty()) {
        std::println("You don't see that here.");
        return;
    }

    player_.inventory.push_back(item_id);
    std::erase(room.item_ids, item_id);
    std::println("You pick up the {}.", items_.at(item_id).name);
}

void Game::do_use(const std::string& arg) {
    if (arg.empty()) {
        std::println("What do you want to use?");
        return;
    }

    auto item_id = find_item_by_name(player_.inventory, arg);
    if (item_id.empty()) {
        std::println("You don't have that.");
        return;
    }

    auto& item = items_.at(item_id);
    if (item.type == ItemType::Potion) {
        player_.hp = std::min(player_.max_hp, player_.hp + item.value);
        std::erase(player_.inventory, item_id);
        std::println("You drink the {}. HP restored to {}/{}.",
                     item.name, player_.hp, player_.max_hp);
    } else {
        std::println("You can't use that directly. Try 'equip' for weapons and armor.");
    }
}

void Game::do_equip(const std::string& arg) {
    if (arg.empty()) {
        std::println("What do you want to equip?");
        return;
    }

    auto item_id = find_item_by_name(player_.inventory, arg);
    if (item_id.empty()) {
        std::println("You don't have that.");
        return;
    }

    auto& item = items_.at(item_id);
    if (item.type == ItemType::Weapon) {
        player_.equipped_weapon = item_id;
        std::println("You equip the {}. (+{} Attack)", item.name, item.value);
    } else if (item.type == ItemType::Armor) {
        player_.equipped_armor = item_id;
        std::println("You equip the {}. (+{} Defense)", item.name, item.value);
    } else {
        std::println("You can't equip that.");
    }
}

int Game::get_bonus_atk() const {
    if (player_.equipped_weapon) {
        auto it = items_.find(*player_.equipped_weapon);
        if (it != items_.end()) return it->second.value;
    }
    return 0;
}

int Game::get_bonus_def() const {
    if (player_.equipped_armor) {
        auto it = items_.find(*player_.equipped_armor);
        if (it != items_.end()) return it->second.value;
    }
    return 0;
}

void Game::do_attack() {
    auto& room = rooms_.at(current_room_id_);
    if (!room.enemy) {
        std::println("There's nothing to attack here.");
        return;
    }

    auto result = run_combat(player_, *room.enemy,
                              get_bonus_atk(), get_bonus_def(), items_);

    switch (result) {
    case CombatResult::Won: {
        auto& loot_id = room.enemy->loot_id;
        if (!loot_id.empty() && items_.contains(loot_id)) {
            room.item_ids.push_back(loot_id);
            std::println("The {} dropped: {}", room.enemy->name, items_.at(loot_id).name);
        }

        // Check if this was the Shadow Lord
        if (room.enemy->id == "shadow_lord") {
            std::println("\n========================================");
            std::println("  VICTORY!");
            std::println("========================================");
            std::println("\nThe Shadow Lord dissolves into darkness.");
            std::println("Light floods the dungeon as the curse is broken.");
            std::println("You are victorious, {}!\n", player_.name);
            std::println("Thanks for playing!");
            running_ = false;
        }

        room.enemy = std::nullopt;
        break;
    }
    case CombatResult::Fled:
        // Enemy stays, player can try again
        break;
    case CombatResult::Died:
        std::println("\n========================================");
        std::println("  GAME OVER");
        std::println("========================================");
        std::println("\nThe dungeon claims another soul...\n");
        running_ = false;
        break;
    }
}

void Game::do_inventory() {
    std::println("--- Inventory ---");
    std::println("HP: {}/{} | ATK: {}+{} | DEF: {}+{}",
                 player_.hp, player_.max_hp,
                 player_.attack, get_bonus_atk(),
                 player_.defense, get_bonus_def());

    if (player_.equipped_weapon) {
        std::println("Weapon: {}", items_.at(*player_.equipped_weapon).name);
    }
    if (player_.equipped_armor) {
        std::println("Armor: {}", items_.at(*player_.equipped_armor).name);
    }

    if (player_.inventory.empty()) {
        std::println("Your pack is empty.");
    } else {
        std::println("Items:");
        for (auto& id : player_.inventory) {
            auto it = items_.find(id);
            if (it != items_.end()) {
                std::println("  - {}: {}", it->second.name, it->second.description);
            }
        }
    }
}

void Game::do_help() {
    std::println("--- Commands ---");
    std::println("  go <direction>  - Move (north/south/east/west, or n/s/e/w)");
    std::println("  look            - Examine your surroundings");
    std::println("  take <item>     - Pick up an item");
    std::println("  use <item>      - Use an item (e.g., potions)");
    std::println("  equip <item>    - Equip a weapon or armor");
    std::println("  attack          - Fight an enemy in the room");
    std::println("  inventory       - Check your stats and items");
    std::println("  help            - Show this message");
    std::println("  quit            - Leave the dungeon");
}
