# Rosario - Devlog - 5

## Table of Contents
1. [Deep in Portlandia](#51---deep-in-portlandia)
2. [Keyboard-Hooking the Mouse-Hooked Menu Navigation](#52-keyboard-hooking-the-mouse-hooked-menu-navigation)
3. [Fun With Squares](#53-fun-with-squares)

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

# 5.2 Keyboard-Hooking the Mouse-Hooked Menu Navigation
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

<br>
<br>

# 5.3 Fun With Squares
The last BIG thing that's still left in the new version is the logo drawing pipelines. In itself, this could be a pretty streamlined process, but there's one tiny problem: the pipeline in the OOP version was already a mess. It had a lot of hardcoded stuff, it wasn't really a pipeline but a lot of manually set things, it already carried visual *cracks* from the initial transition between *nibbler* and the first prototype of *rosario*, ... Way more than enough rubbish to call for a whole redesign. This means that this last part of the port is better thought as an implementation in itself more than a OOP → ECS translation, with the safe net that below it all, the idea in itself is already grounded. So let's start by a general overview and planning phase:
1. The new pipeline needs to closely follow the core principles of the port, i.e., it needs to be layed out with **ECS + Data-Driven + JSON config** in mind.
2. Use cases need to be scoped:
	- static branded text (`ROSARIO`, `GAME OVER`, etc)
	- reusable custom font for arbitrary short UI strings
	- targeted to 2D UI/Menu states only (at least in first milestone)
	- data sourced via JSON glyph library + JSON text presets/layout (DOUBLE JSON!!)
3. Data model:
	- A) Glyph Library JSON
		- Per-character definition using **cell coordinates**
		- Metrics per glyph: `advance`, option `bearingX`, `bearingY`
		- Global defaults: `cellSize`, `letterSpacing`, `lineSpacing`
	- B) Text Preset JSON
		- Named entries: `logo_main`, `gameover_title`, etc
		- Possible fields: `text`, `anchor`, `offset`, `scale`, `palette`, `layer`, `visibleInStates`
	- C) Optional style JSON (TRIPLE JSON????!!!)
		- Color themes (normal, hover, pulse, whateverthehell)
		- Animation tags (fades, pulses, movements, drifts, whoknows)
4. ECS components forseen:
	- `PixelTextComponent`
		- `text`, `glyphLibraryId`, `styleId`, `scale`, `visible`
	- `UITransformComponent`
		- `anchor`, `position`, `z/layer`
	- `PixelTextLayoutComponent` (for cache)
		- precomputed quads and rects for render
		- dirty flags (`textDirty`, `styleDirty`, `transformDirty`)
	- `StateVisibilityComponent` (optional, rreally)
5. ECS systems forseen:
	- `GlyphResourceSystem` (or a loader module, more likely)
		- Just a way to load the JSON based glyph library
	- `PixelTextLayoutSystem`
		- Converting strings into cell quads from glyph definitions
		- Rebuilding only when dirty
	- `PixelTextRenderSystem`
		- The one in charge of drawing quads via `Raylib` (`UI` phase)
	- `PixelTextAnimationSystem`
		- Only if needed down the line
6. Integration (or how to not lose ourselves and our sanity in this process)
	- First vertical slice:
		- One Glyph lib JSON
		- One component + one render system
		- Render only one of the targeted logos in only one state (most likely, `GAME OVER`)
	- Second milestone:
		- Tackle `ROSARIO` in start menu
		- Add anchors and scaling
	- Third milestone:
		- Presets and color themes from JSON
	- Fourth mileston:
		- Animations (most likely not going to happen in this implementation cycle)
		- Caching

What is really paramount during all this process is to visually validate everything that happens in the execution side of things. Glyph order and spacing need to be stable at any resolution, there should not be any frame flashing or broken transitions, all the layering in the menus needs to be wholelly under control, no JSON errors should crash the program.

> *All this pipeline is going to be somewhat, s o m e  w h a t, similar as to what I did back in the day in my [wireframe renderer](https://github.com/hugomgris/FDFPP), which converted string arguments into map based character word building*

So, anyway, a lot of files to be written, so I'll spend the rest of my evening knee deep in this. Nothing new regarding all the porting work that has undergone across logs, but things might get messy when trying to hook up the multi-system nature of what is laid out in the list above.

