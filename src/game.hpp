#pragma once

#include "types.hpp"
#include <map>
#include <string>

class Game {
public:
    Game();
    void run();

private:
    void describe_room();
    void do_go(const std::string& direction);
    void do_look();
    void do_take(const std::string& arg);
    void do_use(const std::string& arg);
    void do_equip(const std::string& arg);
    void do_attack();
    void do_inventory();
    void do_help();

    int get_bonus_atk() const;
    int get_bonus_def() const;
    std::string find_item_by_name(const std::vector<std::string>& ids,
                                   const std::string& name) const;

    Player player_;
    std::map<std::string, Room> rooms_;
    std::map<std::string, Item> items_;
    std::string current_room_id_;
    bool running_ = true;
};
