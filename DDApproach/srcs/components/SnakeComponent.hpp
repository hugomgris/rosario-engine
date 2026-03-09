#ifndef SNAKECOMPONENT_HPP
#define SNAKECOMPONENT_HPP

#include <vector>

struct SnakeComponent {
    int length; // Length of the snake
    std::vector<int> body; // Positions of the snake segments
    int direction; // Current direction of the snake (e.g., up, down, left, right)

    SnakeComponent(int initLength, int initDirection)
        : length(initLength), direction(initDirection) {
        body.resize(length);
        for (int i = 0; i < length; ++i) {
            body[i] = i; // Initialize body positions
        }
    }

    void grow() {
        length++;
        body.push_back(body.back()); // Add a new segment at the end
    }

    void move(int newPosition) {
        // Move the snake by updating the body positions
        for (int i = 0; i < length - 1; ++i) {
            body[i] = body[i + 1];
        }
        body[length - 1] = newPosition; // Update the position of the last segment
    }
};

#endif // SNAKECOMPONENT_HPP