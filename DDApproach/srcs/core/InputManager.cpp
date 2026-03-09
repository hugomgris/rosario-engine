#include "InputManager.hpp"
#include <iostream>
#include <unordered_map>
#include <string>

class InputManager {
public:
    InputManager() {
        // Initialize input mappings
        initializeInputMappings();
    }

    void update() {
        // Update input states
        for (const auto& [action, key] : inputMappings) {
            currentInputStates[action] = isKeyPressed(key);
        }
    }

    bool isActionPressed(const std::string& action) const {
        auto it = currentInputStates.find(action);
        return it != currentInputStates.end() && it->second;
    }

private:
    std::unordered_map<std::string, int> inputMappings;
    std::unordered_map<std::string, bool> currentInputStates;

    void initializeInputMappings() {
        // Map actions to keys
        inputMappings["move_up"] = KEY_W;
        inputMappings["move_down"] = KEY_S;
        inputMappings["move_left"] = KEY_A;
        inputMappings["move_right"] = KEY_D;
        inputMappings["pause"] = KEY_ESCAPE;
        // Add more mappings as needed
    }

    bool isKeyPressed(int key) const {
        return IsKeyPressed(key); // Assuming a function from a library like Raylib
    }
};