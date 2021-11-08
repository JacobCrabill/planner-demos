
/**
 * @File: util.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Miscellaneous globally-useful utility structs and functions
 */
#pragma once

#include <algorithm>
#include <string>
#include <vector>

#include "profile.hpp"
#include "olcPixelGameEngine.h"

#ifdef ENABLE_LIBNOISE
void SetNoiseSeed(int seed);
double GetNoise(double nx, double ny);
#endif

float SimpleRand(int x, int y);

enum PlannerMethod
{
    ASTAR = 0,
    RRTSTAR,
    METHOD_MAX
};

enum MapType
{
    STATIC = 0,
    PROCEDURAL = 1,
    MAPTYPE_MAX
};

/** Struct to contain game input / configuration */
struct Config
{
    std::string fConfig;
    olc::vi2d dims;
    std::vector<uint8_t> map;
    std::vector<float> terrainWeights;
    PlannerMethod method;
    MapType mapType;
    int noiseSeed;
    double noiseScale;
};

MapType MapTypeValFromString(const std::string& maptype);

PlannerMethod MethodValFromString(const std::string& method);

bool LoadInput(const std::string& fname, Config& config);
