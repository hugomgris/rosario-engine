# Rosario - Devlog - 5

## Table of Contents
1. [Deep in Portlandia](#51---deep-in-portlandia)
2. [Keyboard-Hooking the Mouse-Hooked Menu Navigation](#52-keyboard-hooking-the-mouse-hooked-menu-navigation)
3. [Fun With Squares](#53-fun-with-squares)
4. [From Plan to Pipeline](#54---from-plan-to-pipeline)

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

<br>
<br>

# 5.4 - From Plan to Pipeline
Well, the new text pipeline... works!!! While my dinner is being cooked in the air fryer, I seem to have reached Milestone 2, which means that the whole json->custom-text pipeline, all through the ECS structure, is in place and running and awesome. Here's the proof:

<img src="gameover_new.png" alt="new game over logo rendering">

The glyph writing pipeline is now a full working chain in the current build, running in ECS style, data-driven, and with JSON on both shape data and text preset side. The implementation was not linear (what a surprise), because as soon as the first vertical slice was alive, all the real problems started to show up: entity lifetime through registry wipes, spacing semantics, line-break behavior, custom/special glyph usage, connected letters, and final composition alignment. So from here on I'll try to write a full breakdown of what was implemented, why it was done this way, and which bugs were found/fixed along the way.

<br>

## 5.4.1 Vertical Slice: Components + Layout + Render

The first target was intentionally narrow: **render one JSON-defined title in one state** (`GameOver`) using cell-composed glyphs.

Two new components were introduced:

```cpp
struct PixelTextComponent {
	std::string id;
	std::string text;
	Vector2 position {0, 0};
	float scale = 1.0f;
	Color color = {255, 255, 255, 255};
	std::unordered_map<size_t, std::string> glyphOverrides;
	std::unordered_map<size_t, GlyphLigatureOverride> glyphLigatures;
	bool centerX = false;
	bool visible = true;
};

struct PixelTextLayoutComponent {
	std::vector<Rectangle> quads;
	bool dirty = true;
};
```

This gives us the exact split we wanted:
- `PixelTextComponent` = declarative intent (`what` to draw + styling + per-preset behavior)
- `PixelTextLayoutComponent` = cached geometry (`how` it gets drawn)

And the systems follow this split:
- `PixelTextLayoutSystem` builds quads from glyph data only when dirty.
- `PixelTextRenderSystem` draws those quads in the UI phase.

That keeps rendering simple (or as simple as I've been able, which I must say I think is *quite*) and layout deterministic.

<br>

## 5.4.2 JSON Data Sources (Double JSON: confirmed)

Two loaders (red power rangers, because Jason, right???) now feed the pipeline, following the same pattern as any other loader in the new build (i.e., json → struct):

1) `GlyphLibraryLoader` (`data/GlyphLibrary.json`)
- Parses global defaults (`cellSize`, spacing and line metrics)
- Parses glyph map (`cells`, `advance`, optional offsets)
- Supports both char lookup (`'g'`) and named glyph lookup (`"g_logo"`, `"i_snake"`)

2) `GlyphPresetLoader` (`data/GlyphPresets.json`)
- Parses text presets (`id`, `text`, `position`, `scale`, `color`)
- Parses single-position overrides and ligatures
- Parses centering behavior (`centerX`)

Loader-side preset parsing now supports this structure:

```cpp
if (entry.contains("glyphOverrides") && entry.at("glyphOverrides").is_array()) {
	for (const auto& override : entry.at("glyphOverrides")) {
		size_t index = override.at("index").get<size_t>();
		std::string glyphId = override.at("glyph").get<std::string>();
		preset.glyphOverrides[index] = glyphId;
	}
}

if (entry.contains("glyphLigatures") && entry.at("glyphLigatures").is_array()) {
	for (const auto& ligature : entry.at("glyphLigatures")) {
		size_t index = ligature.at("index").get<size_t>();
		GlyphLigatureOverride override;
		override.length = ligature.value("length", static_cast<size_t>(2));
		override.glyph = ligature.at("glyph").get<std::string>();
		preset.glyphLigatures[index] = override;
	}
}
```

This gives us data-driven control over regular/special glyph behavior without hardcoding string-specific cases in C++. More detail about how the handling of regular/special glyphs was implemented later, for now I'll just say that it was, oh yeah, PAINFUL. I had to lay down paths for different definitions of "special", like a letter being two lines tall, or letters being continued with each other (check the screenshot above to see examples for both, "g" and the combination of "am"). Anyways, later, below, keep reading, you'll find it.

<br>

## 5.4.3 The First Big Bug: Segfault After State Changes

The first major crash happened when toggling states around gameplay. Root cause was simple and brutal and *muy yo*. `GameManager::resetGame(...)` wipes the registry (by design, for now), so previously created text entities were destroyed. Then `main` tried to access stale entity IDs:

```cpp
registry = Registry{};
```

```cpp
registry.getComponent<PixelTextComponent>(gameOverTitle)
```

To fix this cleanly, text creation was moved into factory style and rehydration(????) guards were added to make text entities resilient to full-registry resets:

```cpp
Entity ensurePixelTextEntity(Registry& registry, Entity& entity, const PixelTextComponent& templateData) {
	if (registry.hasComponent<PixelTextComponent>(entity)
		&& registry.hasComponent<PixelTextLayoutComponent>(entity)) {
		return entity;
	}

	entity = Factories::spawnPixelText(registry, templateData, true);
	return entity;
}
```

<br>

## 5.4.4 Layout Semantics: Cell Space vs Pixel Space

A second bug appeared as piled-up glyphs (letters overlapping). The main cause was the classic mixeroo: horizontal advance was partially computed in cell units and partially in pixels, causing a slightly offset overlap, like a pile of letters that was somewhat still displacing each letter a little bit to the right. The corrected behavior is:

```cpp
cursorX += ((advanceCells * cellPixel) + letterSpacingPixel) * text.scale;
```

where:
- `advanceCells` = glyph width in cells
- `cellPixel` = `cellSize`
- `letterSpacingPixel` = fixed separator from JSON

That restored stable horizontal spacing, resolution-independent under scaling.

<br>

## 5.4.5 Special Glyphs Without Breaking Regular Typography

As advanced in a previous section, another classic problem emerged immediately: some letters need to be special only in specific words (`g` in logo, snake-tail `i`), but regular elsewhere. It's not like I have a wide array of current cases that requires a very detailed handling of specific choices, to be honest, but 1) I want to make things as *right* and *scalable* and *cool* as possible, and 2) you never know. I can now write "game over" with a regular 'g' or a custom, logo-specific character, same for the continued 'a'+'m'. To do so, two types of excepcions/overrides were put in place, a direct **override-by-index** and a **ligature**, horizontal-wise alternative possibility.

```json
"glyphOverrides": [
	{ "index": 0, "glyph": "g_logo" }
]
```

Layout resolution order is now:
1. per-index ligature override
2. per-index single glyph override
3. default char glyph lookup

In other words, an `'i'` character is a regular `'i' unless explicitly overridden in a given preset.

<br>

## 5.4.6 Connected Letters (Ligatures)

As I said, but want to have in its specific subsection, to support continuous shapes like `am` in `game`, single-char overrides were not enough. A ligature layer was added:

```cpp
struct GlyphLigatureOverride {
	size_t length = 2;
	std::string glyph;
};
```

Preset example:

```json
"glyphLigatures": [
	{ "index": 1, "length": 2, "glyph": "am_join" }
]
```

At layout time, if a ligature matches current text position (and does not cross a line break), one glyph is used and the corresponding number of source characters are consumed. This allows connected typography while keeping source text readable and editable. I had to tweak this A LOT until it worked, but hey, it works, so: nice.

<br>

## 5.4.7 Newline + Leading Space Edge Case

Another visual edge case surfaced when using `"\n "` patterns: second-line first letter could appear visually glued to descender shapes. The layout pass now tracks line start and leading spaces and injects separator intent consistently before first non-space glyph on that line. This was necessary to make intentional indentation (`\n over`) behave as authored instead of being collapsed by visual overlap assumptions.

<br>

## 5.4.8 Vertical Advance: Removing Double Gap Feel

A subtle but important fix: the second line started one cell too low. The issue came from effectively over-counting vertical spacing logic. The current newline step is now strict and predictable, and the vertical separator behavior is adjusted to match the horizontal indent (`separator == one cell`) in the current configuration:

```cpp
cursorY += (lineAdvanceCells * cellPixel) * text.scale;
```

with default line advance resolving to:

```cpp
return capHeightCells + lineGapCells;
```

<br>

## 5.4.9 Centering the Whole Composition

After glyph correctness, composition alignment was the final pass. The title was shifted right, so centering support was added directly in the pipeline.

Preset-side flag:

```json
"centerX": true
```

Layout-side adjustment (post-quad generation):

```cpp
const float currentCenterX = (minX + maxX) * 0.5f;
const float deltaX = text.position.x - currentCenterX;
for (auto& quad : layout.quads) {
	quad.x += deltaX;
}
```

Because centering is done on produced geometry, it naturally includes:
- glyph overrides,
- ligatures,
- spaces,
- and mixed-width glyphs.

No separate width precomputation pass needed.

<br>

## 5.4.10 Main Loop Integration (Current State)

`main` now does all glyph text work in UI phase, state-scoped and with safe entity rehydration:
- In `Menu`:
	- `menu_logo` is ensured and shown (if preset exists)
	- `gameover_title` is ensured and hidden
- In `GameOver`:
	- `gameover_title` is ensured and shown
	- `menu_logo` is ensured and hidden (if preset exists)

Then a single `pixelTextLayoutSystem.update(registry, glyphLib)` runs before drawing, and `pixelTextRenderSystem.render(registry)` draws in UI pass.

This keeps pixel text where it belongs in the frame graph: after world render, inside UI composition, before post-processing present.

<br>

## 5.4.11 What Is Done vs What Is Next

Done in this cycle:
- ECS componentized pixel text pipeline
- JSON glyph lib + JSON preset loading
- Dirty layout caching and render split
- Registry-reset-safe text entity lifecycle
- Cell/pixel spacing correction
- Per-index special glyph overrides
- Ligatures for connected letters
- Newline-leading-space handling improvements
- Line advance correction
- Full-composition horizontal centering
- Optional `menu_logo` preset integration

Likely next steps:
- score/result dynamic text through same pipeline
- optional `centerY` / block anchor modes
- preset/state filtering helper (currently visibility is state-managed in `main`)
- optional hot reload for glyph jsons during iteration


