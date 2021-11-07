
/**
 * @File: util.cpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 *
 * @Description:
 *     Miscellaneous globally-useful utility structs and functions
 */
#include "util.hpp"

#include "yaml-cpp/yaml.h"

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
#endif // ENABLE_LIBNOISE

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

bool LoadInput(const std::string& fname, Config& config)
{
    YAML::Node input;
    try {
        input = YAML::LoadFile(fname);
    } catch (const std::exception& e) {
        // I wouldn't normally do this, but YAML-Cpp's error messages aren't as useful as I'd like
        std::cout << "Error parsing input file: " << fname << std::endl;
        std::cout << "    Check that the relaive file path is correct, and that the YAML syntax is valid." << std::endl;
        std::cout << "    Exception Message: '" << e.what() << "'" << std::endl << std::endl;;
        return false;
    }

    config.fConfig = fname;
    config.dims.x = input["dims"]["x"].as<int>();
    config.dims.y = input["dims"]["y"].as<int>();
    config.mapType = MapTypeValFromString(input["maptype"].as<std::string>());
    config.method = MethodValFromString(input["method"].as<std::string>());

    if (config.mapType == MapType::STATIC) {
        if (input["map"]) {
            config.map = input["map"].as<std::vector<int>>();
        } else {
            std::cout << "Static map requested but map not given" << std::endl;
            exit(1);
        }

    } else {
        // --- Configuration for Procedural Map Generation ---
        config.noiseSeed = 0.;
        if (input["noiseSeed"]) {
            config.noiseSeed = input["noiseSeed"].as<int>();
        }

        config.noiseScale = 5.;
        if (input["noiseScale"]) {
            config.noiseScale = input["noiseScale"].as<double>();
        }

        config.terrainWeights = {.4f, .2f, .2f, .1, .1f};
        if (input["terrainWeights"]) {
            config.terrainWeights = input["terrainWeights"].as<std::vector<float>>();
        }
    }

    return true;
}
