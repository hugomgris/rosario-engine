#include "MenuSystem.hpp"
#include "RaylibColors.hpp"
#include "GameConfig.hpp"
#include <iostream>

MenuSystem::MenuSystem() {
    // Load menu configurations from JSON or other data sources
    loadMenuConfig();
}

void MenuSystem::loadMenuConfig() {
    // Load menu configuration data from a JSON file or other sources
    // This is where you would parse the JSON and populate menu items
}

void MenuSystem::renderMenu() {
    // Clear the screen
    ClearBackground(RAYWHITE);

    // Draw menu title
    DrawText("Snake Game Menu", 100, 100, 40, BLACK);

    // Draw menu options
    for (size_t i = 0; i < menuItems.size(); ++i) {
        DrawText(menuItems[i].c_str(), 100, 200 + i * 40, 20, DARKGRAY);
    }

    // Draw selected option highlight
    if (selectedItem < menuItems.size()) {
        DrawRectangle(80, 200 + selectedItem * 40, 200, 30, LIGHTGRAY);
    }
}

void MenuSystem::updateMenu() {
    // Handle input for menu navigation
    if (IsKeyPressed(KEY_DOWN)) {
        selectedItem = (selectedItem + 1) % menuItems.size();
    }
    if (IsKeyPressed(KEY_UP)) {
        selectedItem = (selectedItem + menuItems.size() - 1) % menuItems.size();
    }
    if (IsKeyPressed(KEY_ENTER)) {
        // Handle menu selection
        handleMenuSelection();
    }
}

void MenuSystem::handleMenuSelection() {
    // Perform action based on selected menu item
    switch (selectedItem) {
        case 0:
            std::cout << "Start Game Selected" << std::endl;
            // Start the game
            break;
        case 1:
            std::cout << "Options Selected" << std::endl;
            // Open options menu
            break;
        case 2:
            std::cout << "Exit Selected" << std::endl;
            // Exit the game
            break;
        default:
            break;
    }
}