#include "../../incs/Pathfinder.hpp"
#include "../../incs/Snake.hpp"

void PathFinder::cleanAllNodes(std::vector<Node*> allNodes) {
	for (Node* node : allNodes) {
        delete node;
    }
}

std::vector<Vec2> PathFinder::reconstructPath(Node *goalNode) {
	std::vector<Vec2> path;
	Node *current = goalNode;

	// main idea -> follow parent nodes backwards
	while (current != nullptr && current->parent != nullptr) {
		path.push_back(current->pos);
		current = current->parent;
	}

	std::reverse(path.begin(), path.end());

	return path;
}

// TODO: snake needs to detect obstacles and growths in its pathfinding
std::vector<Vec2> PathFinder::findPath(const GameState &state, Vec2 start, Vec2 goal, int maxDepth) {
	std::multiset<Node*, CompareNode> openList;
	std::vector<std::vector<bool>> visited(state.width, std::vector<bool>(state.height, false));
	std::vector<Node*> allNodes;	//needed for clenaup

	Node *startNode = new Node {
		start,
		0,
		manhattanDistance(start, goal),
		nullptr
	};

	openList.insert(startNode);
	allNodes.push_back(startNode);
	int nodesExplored = 0;

	while (!openList.empty() && nodesExplored < maxDepth) {
		nodesExplored++;

		Node *current = *openList.begin();
		openList.erase(openList.begin()); // take the begin node because CompareNode sorts ascending, i.e. the one with lowest f-cost

		if (current->pos.x == goal.x && current->pos.y == goal.y) {
			// found the food
			std::vector<Vec2> path = reconstructPath(current);

			cleanAllNodes(allNodes);

			return path;
		}

		visited[current->pos.x][current->pos.y] = true;

		std::vector<Vec2> neighbors = getNeighbors(current->pos);

		for (Vec2 neighborPos : neighbors) {
			// skip non walkable and visited positions
			if (!isWalkable(state, neighborPos)) continue;
			if (visited[neighborPos.x][neighborPos.y]) continue;

			int tentativeGCost = current->gCost + 1;	// minimum cost would be one step
			int hCost = manhattanDistance(neighborPos, goal);

			Node * neighborNode = new Node {
				neighborPos,
				tentativeGCost,
				hCost,
				current		//parent is current node
			};

			openList.insert(neighborNode);
			allNodes.push_back(neighborNode);
		}
	}

	cleanAllNodes(allNodes);

	return {};
}