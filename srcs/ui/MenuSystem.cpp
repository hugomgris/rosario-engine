#include "MenuSystem.hpp"
#include "../../incs/RaylibColors.hpp"

MenuSystem::MenuSystem(int sWidth, int sHeight) : _screenWidth(sWidth), _screenHeight(sHeight) {}

void MenuSystem::buildUI(FrameContext &ctx, UIRenderQueue& queue) {
    UITextCmd placeHolder = {
        "S T A R T",
        _screenWidth / 2.0f,
        _screenHeight / 2.0f,
        20,
        customWhite,
        true
    };
    
    queue.texts.push_back(placeHolder);
}