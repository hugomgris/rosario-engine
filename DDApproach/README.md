# Rosario Engine

## Overview
The Rosario Engine is a data-driven game engine designed for a snake game, utilizing an Entity-Component-System (ECS) architecture. This architecture allows for flexible and modular game development, enabling easy management of game entities and their behaviors.

## Features
- **Entity-Component-System (ECS) Architecture**: Separates data (components) from behavior (systems), promoting a clean and maintainable codebase.
- **Modular Components**: Each game entity can have multiple components, such as Transform, Snake, Collision, Render, AI, and Food components, allowing for diverse entity behaviors.
- **Dynamic Systems**: Systems like Movement, Collision, Render, AI, and Animation handle the logic and rendering of entities based on their components.
- **Configurable Data**: Game settings, arena configurations, and AI behaviors are defined in JSON files, allowing for easy adjustments without modifying the code.

## Project Structure
```
rosario-engine
├── srcs
│   ├── main.cpp
│   ├── components
│   ├── systems
│   ├── ecs
│   ├── arena
│   ├── data
│   ├── AI
│   ├── graphics
│   └── core
├── configs
├── docs
├── Makefile
└── README.md
```

## Getting Started
1. **Clone the Repository**: 
   ```
   git clone <repository-url>
   cd rosario-engine
   ```

2. **Build the Project**: 
   Use the provided Makefile to compile the project.
   ```
   make
   ```

3. **Run the Game**: 
   After building, run the executable to start the game.

## Configuration Files
- **arena_default.json**: Default settings for the game arena.
- **arena_presets.json**: Predefined arena configurations for various game modes.
- **ai_config.json**: Configuration settings for AI behavior.
- **game_config.json**: General game settings, including speed and difficulty.

## Documentation
For more detailed information on the ECS architecture and implementation, refer to the `docs/ecs_architecture.md` file. Additionally, the `docs/v2_devlog_02.md` contains development logs detailing the progress and changes made to the engine.

## Contributing
Contributions are welcome! Please submit a pull request or open an issue for any enhancements or bug fixes.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.