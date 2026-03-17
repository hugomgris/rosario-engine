#pragma once

#include <string>
#include <functional>
#include <raylib.h>
#include "UIQueue.hpp"
#include "ButtonConfig.hpp"

struct Button {
	Rectangle				bounds;
	ButtonConfig			config;
	std::function<void()>	onClick;
	bool					hovered = false;

	bool isHovered(Vector2 mousePos) const {
		return CheckCollisionPointRec(mousePos, bounds);
	}

	void enqueueRenderCommands(UIRenderQueue& queue) const {
		Color bg		= hovered ? config.hoverColor : config.backgroundColor;
		Color outline	= hovered ? config.outlineHoverColor : config.outlineColor;
		Color text		= hovered ? config.textHoverColor  : config.textColor;

		queue.rects.push_back(UIRectCmd{
			bounds,
			bg,
			false,
			config.outlineThickness
		});

		queue.rects.push_back(UIRectCmd{
			bounds,
			outline,
			true,
			config.outlineThickness
		});

		queue.texts.push_back(UITextCmd{
			config.label,
			bounds.x + bounds.width * 0.5f,
			bounds.y + bounds.height * 0.5f,
			config.fontSize,
			text,
			true
		});
	}
};