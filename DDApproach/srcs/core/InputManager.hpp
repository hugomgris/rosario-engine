#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include <map>
#include <string>
#include <vector>
#include "ecs/Entity.hpp"

class InputManager {
public:
    InputManager();
    void update();
    void bindAction(const std::string& action, int key);
    bool isActionPressed(const std::string& action) const;
    bool isActionReleased(const std::string& action) const;
    void clearActions();

private:
    std::map<std::string, int> actionBindings;
    std::map<std::string, bool> actionStates;
    std::map<std::string, bool> previousActionStates;

    void processInput();
};

#endif // INPUTMANAGER_HPP