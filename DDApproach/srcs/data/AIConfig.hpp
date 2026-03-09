#pragma once

#include <string>
#include <unordered_map>

class AIConfig {
public:
    AIConfig(const std::string& configFilePath);

    float getDecisionMakingParameter(const std::string& parameterName) const;
    int getBehaviorState(const std::string& stateName) const;

private:
    void loadConfig(const std::string& configFilePath);

    std::unordered_map<std::string, float> decisionMakingParameters;
    std::unordered_map<std::string, int> behaviorStates;
};