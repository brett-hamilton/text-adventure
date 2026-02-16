#include "parser.hpp"
#include <algorithm>
#include <sstream>

static std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

Command parse_command(const std::string& raw_input) {
    // Truncate to max 256 chars
    std::string input = raw_input.substr(0, 256);
    input = to_lower(trim(input));

    if (input.empty()) {
        return {CommandType::Unknown, ""};
    }

    // Split into words (handles multiple spaces)
    std::istringstream iss(input);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }

    if (words.empty()) {
        return {CommandType::Unknown, ""};
    }

    const auto& verb = words[0];

    // Build argument from remaining words
    std::string arg;
    for (size_t i = 1; i < words.size(); ++i) {
        if (!arg.empty()) arg += ' ';
        arg += words[i];
    }

    // Direction shortcuts: bare direction without "go"
    if (verb == "n" || verb == "north") return {CommandType::Go, "north"};
    if (verb == "s" || verb == "south") return {CommandType::Go, "south"};
    if (verb == "e" || verb == "east") return {CommandType::Go, "east"};
    if (verb == "w" || verb == "west") return {CommandType::Go, "west"};

    if (verb == "go") return {CommandType::Go, arg};
    if (verb == "look" || verb == "l") return {CommandType::Look, ""};
    if (verb == "take" || verb == "get" || verb == "pick") return {CommandType::Take, arg};
    if (verb == "use") return {CommandType::Use, arg};
    if (verb == "equip" || verb == "wear") return {CommandType::Equip, arg};
    if (verb == "attack" || verb == "fight" || verb == "kill") return {CommandType::Attack, ""};
    if (verb == "inventory" || verb == "i") return {CommandType::Inventory, ""};
    if (verb == "help" || verb == "?") return {CommandType::Help, ""};
    if (verb == "quit" || verb == "exit" || verb == "q") return {CommandType::Quit, ""};

    return {CommandType::Unknown, ""};
}
