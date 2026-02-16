#include "data_loader.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <stdexcept>

using json = nlohmann::json;

static ItemType parse_item_type(const std::string& s) {
    if (s == "Potion") return ItemType::Potion;
    if (s == "Weapon") return ItemType::Weapon;
    if (s == "Armor") return ItemType::Armor;
    if (s == "Key") return ItemType::Key;
    throw std::runtime_error("Unknown item type: " + s);
}

static std::optional<Enemy> parse_enemy(const json& j) {
    if (j.is_null()) return std::nullopt;
    Enemy e;
    e.id = j.at("id");
    e.name = j.at("name");
    e.hp = j.at("hp");
    e.max_hp = e.hp;
    e.attack = j.at("attack");
    e.defense = j.at("defense");
    e.loot_id = j.value("loot_id", "");
    return e;
}

static json read_json(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    return json::parse(f);
}

GameData load_game_data(const std::string& data_dir) {
    GameData data;

    // Load items
    auto items_json = read_json(data_dir + "/items.json");
    for (auto& j : items_json) {
        Item item;
        item.id = j.at("id");
        item.name = j.at("name");
        item.description = j.at("description");
        item.type = parse_item_type(j.at("type"));
        item.value = j.at("value");
        data.items[item.id] = std::move(item);
    }

    // Load rooms
    auto rooms_json = read_json(data_dir + "/rooms.json");
    data.start_room = rooms_json.at("start_room");
    for (auto& j : rooms_json.at("rooms")) {
        Room room;
        room.id = j.at("id");
        room.name = j.at("name");
        room.description = j.at("description");
        for (auto& [dir, target] : j.at("exits").items()) {
            room.exits[dir] = target;
        }
        for (auto& item_id : j.at("item_ids")) {
            room.item_ids.push_back(item_id);
        }
        room.enemy = parse_enemy(j.at("enemy"));
        data.rooms[room.id] = std::move(room);
    }

    return data;
}
