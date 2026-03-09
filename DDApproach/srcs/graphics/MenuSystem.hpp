#pragma once

#include <string>
#include <vector>
#include "ecs/Entity.hpp"

class MenuSystem {
public:
    MenuSystem();
    ~MenuSystem();

    void initialize();
    void update(float deltaTime);
    void render();
    void handleInput();

    void addMenuItem(const std::string& item);
    void setSelectedItem(int index);
    int getSelectedItem() const;

private:
    std::vector<std::string> menuItems;
    int selectedItemIndex;
    Entity menuEntity; // Entity representing the menu in the ECS
};