#ifndef FOODCOMPONENT_HPP
#define FOODCOMPONENT_HPP

#include <cstdint>

class FoodComponent {
public:
    FoodComponent(uint32_t nutritionalValue = 1);

    uint32_t getNutritionalValue() const;
    void setNutritionalValue(uint32_t value);

private:
    uint32_t nutritionalValue; // Nutritional value of the food
};

#endif // FOODCOMPONENT_HPP