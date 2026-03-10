#pragma once

#include "DataStructs.hpp"

// A* search node
struct Node {
    Vec2  pos;
    int   gCost;  // cost from start
    int   hCost;  // heuristic to goal
    Node* parent;

    int fCost() const { return gCost + hCost; }
};

struct CompareNode {
    bool operator()(const Node* a, const Node* b) const {
        if (a->fCost() != b->fCost())
            return a->fCost() < b->fCost();
        return a->hCost < b->hCost; // tie-break: prefer closer to goal
    }
};
