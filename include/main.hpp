#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define WIDTH 640
#define HEIGHT 480

#include "tilemap.hpp"
#include "astar.hpp"

// Override base class with your custom functionality
class AstarDemo : public olc::PixelGameEngine
{
public:
    AstarDemo()
    {
        // Name your application
        sAppName = "AstarDemo";
    }

    olc::Renderable rHighlight;

    olc::vi2d vGoalIJ;
    olc::vi2d vStartIJ;
    olc::vf2d vGoalPos;
    bool isGoalSet {false};

    const olc::vf2d noscale = {1.f, 1.f};

    AStar astar;
    TileMap tileMap;

    uint8_t layerBG;
    uint8_t layerGame;

public:
    bool OnUserCreate() override;

    bool OnUserUpdate(float fElapsedTime) override;

    void DrawBackground();
};
