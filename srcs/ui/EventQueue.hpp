#pragma once

#include <vector>
#include "../components/ButtonActionComponent.hpp"

struct GameEvent {
    enum Type {
        ButtonClicked,
        ModeChanged,
        Quit,
        ReturnToMenu
    };
    
    Type type;
    ButtonActionType buttonAction;
};

class EventQueue {
private:
    std::vector<GameEvent> _events;

public:
    EventQueue() = default;

    void enqueue(const GameEvent& event) {
        _events.push_back(event);
    }

    const std::vector<GameEvent>& getEvents() const {
        return _events;
    }

    void clear() {
        _events.clear();
    }

    bool isEmpty() const {
        return _events.empty();
    }
};
