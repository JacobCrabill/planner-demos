
/**
 * @File: util.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Miscellaneous globally-useful utility structs and functions
 */
#include "util.hpp"

#ifdef ENABLE_LIBNOISE
#include "noise/noise.h"

noise::module::Perlin gen;

void SetNoiseSeed(int seed)
{
    gen.SetSeed(seed);
}

double GetNoise(double nx, double ny)
{
  // Rescale from -1.0:+1.0 to 0.0:1.0
  return gen.GetValue(nx, ny, 0) / 2.0 + 0.5;
}
#endif

MapType MapTypeValFromString(const std::string& maptype)
{
    std::string m = maptype;
    std::transform(m.begin(), m.end(), m.begin(), ::tolower);

    if (m == "static") return MapType::STATIC;
    if (m == "procedural") return MapType::PROCEDURAL;

    return MapType::MAPTYPE_MAX;
}

PlannerMethod MethodValFromString(const std::string& method)
{
    std::string m = method;
    std::transform(m.begin(), m.end(), m.begin(), ::tolower);

    if (m == "a*" || m == "astar") return PlannerMethod::ASTAR;
    if (m == "rrt*" || m == "rrtstar") return PlannerMethod::RRTSTAR;

    return PlannerMethod::METHOD_MAX;
}
