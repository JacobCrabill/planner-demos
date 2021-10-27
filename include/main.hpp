/**
 * @File: main.hpp
 * @Author: Jacob Crabill <github.com/JacobCrabill>
 */
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#include "astar.hpp"
#include "util.hpp"
#include "gamemap.hpp"

// Override base class with your custom functionality
class AstarDemo : public olc::PixelGameEngine
{
public:
    AstarDemo(const Config& _config) :
        gameMap(_config)
    {
        // Name your application
        sAppName = "AstarDemo";
        config = _config;
    }

protected:
    bool OnUserCreate() override;

    bool OnUserUpdate(float fElapsedTime) override;

    void DrawBackground();

private:
    olc::Renderable tileHighlight;

    olc::vi2d goalIJ;
    olc::vi2d startIJ;
    olc::vf2d goalPos;
    bool isGoalSet {false};
    bool havePath {false};

    const olc::vf2d noscale = {1.f, 1.f};

    AStar astar;
    GameMap gameMap;
    Config config;

    uint8_t layerBG;
    uint8_t layerGame;
};
