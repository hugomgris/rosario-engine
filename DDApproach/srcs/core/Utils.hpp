#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Utils {
public:
    static json loadJson(const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return json();
        }

        json j;
        file >> j;
        return j;
    }

    static void saveJson(const std::string& filePath, const json& j) {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return;
        }

        file << j.dump(4); // Pretty print with 4 spaces
    }

    static std::vector<std::string> splitString(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
    }
};

#endif // UTILS_HPP