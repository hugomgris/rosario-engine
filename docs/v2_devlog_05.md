# Rosario - Devlog - 5

## Table of Contents
1. [Deep in Portlandia](#51---deep-in-portlandia)
2. [Keyboard-Hooking the Mouse-Hooked Menu Navigation](#52-keyboard-hooking-the-mouse-hooked-menu-navigation)

<br>
<br>


# 5.1 - Deep in Portlandia
This is going to be the log were the porting ends. Most of it is already done, but the following are pending, all related to the menus:
- [x] Keyboard co-navigation of the buttons
- Logos (for which I want to build a specific pipeline)
- Some more particles in the start menu
- Some way of displaying extra info both in start (current mode) and in gameover (score/results)

Besides that, the OOP version also had a hooked up food-eating->arena-changing chain of effects, but it was extremely placeholder-y and we can just set that as the new developent starting point, i.e. not port-related, but just the entry point for the next phase of development (full in ECS/DataDriven land). Let's get moving!

<br>

### 5.2 Keyboard-Hooking the Mouse-Hooked Menu Navigation
Back in OOP, mouse menu navigation was optional, not forced. We need to take that to the current build, so that it works the same:
- Arrows up and down to navigate, setting the hovered button to the navigated one
- Enter to interact with buttons (same as the current left mouse click)
- Selection cycling (i.e. moving past the last button takes you to the first one)

This is now wired in ECS with one interaction flow, not two separate ones:
- Every frame in `Menu` and `GameOver`, `main.cpp` calls `UIInteractionSystem::update(registry, eventQueue, activeMenu)`.
- `activeMenu` is explicitly passed (`ButtonMenu::Start` or `ButtonMenu::GameOver`) so only relevant buttons are considered.
- The system builds a deterministic list of candidate buttons by filtering per menu and sorting by `ButtonConfig.index`.

Key behavior now:
- `Down` / `S`: move hover forward.
- `Up` / `W`: move hover backward.
- No current hover + `Down`/`S`: select first button.
- No current hover + `Up`/`W`: select last button.
- Wrap-around stays enabled for full cyclic navigation.
- `Enter`/`Numpad Enter`: dispatches the same `ButtonClicked` event path as mouse clicks.

Mouse coexistence behavior:
- Hover chosen by keyboard persists while mouse is idle.
- If mouse moves or clicks, mouse takes ownership of hover for that frame.
- Left click dispatches `ButtonClicked` exactly like before.
- Non-active menu buttons have hover cleared to avoid cross-menu state leakage.

Small UX alias added for start menu mode cycling:
- `Left`/`Right` and `A`/`D` trigger the `modeChange` button action directly in the start menu.
- When this happens, hover is also moved onto the mode button for visual coherence.

Keyboard and mouse are now truly co-navigating the same ECS button state, instead of competing through different code paths. We're almost port-done!


