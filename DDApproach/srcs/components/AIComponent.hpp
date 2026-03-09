#ifndef AICOMPONENT_HPP
#define AICOMPONENT_HPP

#include <string>
#include <vector>

class AIComponent {
public:
    enum class AIState {
        Idle,
        Moving,
        Attacking,
        Fleeing
    };

    AIComponent();
    AIComponent(AIState initialState, float decisionInterval);

    void update(float deltaTime);
    void setState(AIState newState);
    AIState getState() const;

    void setDecisionParameters(const std::vector<std::string>& parameters);
    const std::vector<std::string>& getDecisionParameters() const;

private:
    AIState currentState;
    float decisionTimer;
    float decisionInterval;
    std::vector<std::string> decisionParameters;

    void makeDecision();
};

#endif // AICOMPONENT_HPP