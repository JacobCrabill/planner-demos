
/**
 * @File: util.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Miscellaneous globally-useful utility structs and functions
 */
#pragma once

#include <string>
#include <algorithm>

#include "profile.hpp"
#include "olcPixelGameEngine.h"

enum PlannerMethod
{
    ASTAR = 0,
    RRTSTAR,
    METHOD_MAX
};

static
PlannerMethod MethodValFromString(const std::string& method)
{
    std::string m = method;
    std::transform(m.begin(), m.end(), m.begin(), ::tolower);

    if (m == "a*" || m == "astar") return PlannerMethod::ASTAR;
    if (m == "rrt*" || m == "rrtstar") return PlannerMethod::RRTSTAR;

    return PlannerMethod::METHOD_MAX;
}

/** Struct to contain game input / configuration */
struct Config
{
    std::string fConfig;
    olc::vi2d dims;
    std::string sMap;
    PlannerMethod method;
};
