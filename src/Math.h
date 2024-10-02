#pragma once

#include <cstdlib>
#include <utility>

namespace drive
{
class Math
{
  public:
    static double RandomDouble()
    {
        return static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX);
    }

    static double RandomDouble(double min, double max)
    {
        if (min > max)
        {
            std::swap(min, max);
        }
        return min + RandomDouble() * (max - min);
    }

    static float RandomFloat()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    static float RandomFloat(float min, float max)
    {
        if (min > max)
        {
            std::swap(min, max);
        }
        return min + RandomFloat() * (max - min);
    }
};
}; // namespace drive
