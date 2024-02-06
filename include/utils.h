#include <random>

uint32_t getSpreadDelay(uint32_t delayMs)
{
    std::random_device rd;
    
    std::mt19937 gen(rd());
    
    std::uniform_int_distribution<int16_t> distribution(-1000, 1000);

    int16_t randomNumber = distribution(gen);
    
    return delayMs + randomNumber;
}