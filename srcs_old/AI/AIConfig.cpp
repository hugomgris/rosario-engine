#include "../../incs/DataStructs.hpp"

// difficulty presets
AIConfig AIConfig::easy() {
    return {
        .level = EASY,
        .thinkDelay = 2,			// slow reaction
        .maxSearchDepth = 50,		// shallow search
        .useSafetyCheck = false,	// no planning
        .predictOpponent = false,	// no predicting
        .randomMoveChance = 0.15f,	// 15% random
        .aggressiveness = 0.8f,		// greedy boi
		.hasSurvivalMode = false	// doesn't chase own tail
    };
}

AIConfig AIConfig::medium() {
	return {
		.level = MEDIUM,
		.thinkDelay = 0,
		.maxSearchDepth = 150,
		.useSafetyCheck = true,		// Basic safety
		.predictOpponent = false,
		.randomMoveChance = 0.0f,
		.aggressiveness = 0.5f,		// Chill boi
		.hasSurvivalMode = true		// doesn't chase own tail
	};
}

AIConfig AIConfig::hard() {
	return {
		.level = HARD,
		.thinkDelay = 0,
		.maxSearchDepth = 300,
		.useSafetyCheck = true,
		.predictOpponent = true,	// smart
		.randomMoveChance = 0.0f,
		.aggressiveness = 0.3f,		// cautious boi
		.hasSurvivalMode = true		// doesn't chase own tail
	};
}
