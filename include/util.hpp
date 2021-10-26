
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

/** Struct to contain game input / configuration */
struct Config
{
    std::string fConfig;
    olc::vi2d dims;
};
