# Rosario - Devlog - 2

## Table of Contents
1. [Transition to Data-Driven Architecture](#21---transition-to-data-driven-architecture)
2. [Refactoring Components and Systems](#22---refactoring-components-and-systems)
3. [Gameplay Configuration](#23---gameplay-configuration)

<br>
<br>

# 2.1 - Transition to Data-Driven Architecture
As part of the ongoing development of the `Rosario` game engine, a significant shift has been made towards a data-driven architecture. This change aligns with industry standards and is particularly relevant for my application for a gameplay programmer internship at Larian Barcelona. The goal is to decouple game logic from the underlying data structures, allowing for greater flexibility and easier modifications in gameplay mechanics.

The transition involves restructuring how entities, components, and systems interact. Instead of tightly coupling game logic within classes, we now define behaviors and properties through data files and configuration settings. This allows designers to tweak gameplay elements without needing to modify the codebase directly.

<br>
<br>

# 2.2 - Refactoring Components and Systems
The existing components have been refactored to fit into the new architecture. Each component now represents a specific aspect of an entity's behavior or state, such as movement, rendering, and AI. The components are defined in header files, and their data is stored in a more structured format, allowing for easy access and modification.

For example, the `SnakeComponent` now holds data such as length and direction, while the `TransformComponent` manages position, rotation, and scale. This separation of concerns makes it easier to manage and update entity behaviors through the new ECS (Entity-Component-System) framework.

Additionally, systems have been implemented to handle the logic associated with these components. The `MovementSystem`, for instance, processes all entities with a `TransformComponent` and updates their positions based on input or AI decisions. This modular approach enhances the maintainability of the code and allows for more complex interactions between entities.

<br>
<br>

# 2.3 - Gameplay Configuration
To facilitate the data-driven approach, various gameplay configurations have been introduced. These configurations are stored in JSON files, allowing for easy adjustments to game parameters without recompiling the code. For instance, the `ai_config.json` file contains settings for AI behavior, such as decision-making thresholds and movement speeds.

The `arena_default.json` and `arena_presets.json` files define the arena's dimensions, wall types, and other environmental factors. This setup empowers designers to create diverse gameplay experiences by simply modifying these configuration files, making the development process more efficient and responsive to feedback.

In conclusion, the transition to a data-driven architecture marks a pivotal step in the evolution of the `Rosario` game engine. This approach not only aligns with industry practices but also enhances the overall flexibility and scalability of the project, paving the way for future developments and innovations.