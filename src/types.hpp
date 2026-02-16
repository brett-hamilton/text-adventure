#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

enum class ItemType { Potion, Weapon, Armor, Key };

struct Item {
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    int value = 0;
};

struct Enemy {
    std::string id;
    std::string name;
    int hp = 0;
    int max_hp = 0;
    int attack = 0;
    int defense = 0;
    std::string loot_id;
};

struct Room {
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, std::string> exits;
    std::vector<std::string> item_ids;
    std::optional<Enemy> enemy;
};

struct Player {
    std::string name;
    int hp = 100;
    int max_hp = 100;
    int attack = 10;
    int defense = 5;
    std::vector<std::string> inventory;
    std::optional<std::string> equipped_weapon;
    std::optional<std::string> equipped_armor;
};

enum class CommandType {
    Go,
    Look,
    Take,
    Use,
    Equip,
    Attack,
    Inventory,
    Help,
    Quit,
    Unknown
};

struct Command {
    CommandType type = CommandType::Unknown;
    std::string argument;
};

struct GameData {
    std::map<std::string, Item> items;
    std::map<std::string, Room> rooms;
    std::string start_room;
};
