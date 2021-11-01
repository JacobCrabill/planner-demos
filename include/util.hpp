
/**
 * @File: util.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Miscellaneous globally-useful utility structs and functions
 */
#pragma once

#include <string>

#include "olcPixelGameEngine.h"

#ifdef INCLUDE_PROFILING
    #include "profile.hpp"
    #define PROFILE_FUNC() InstrumentationTimer timer(__PRETTY_FUNCTION__)
    #define PROFILE(s) InstrumentationTimer timer(s)
#else
    #define PROFILE_FUNC()
    #define PROFILE(s)
#endif // INCLUDE_PROFILING

/** Struct to contain game input / configuration */
struct Config
{
    std::string fConfig;
    olc::vi2d dims;
    std::string sMap;
};
