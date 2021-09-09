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

    uint8_t layerBG;
    uint8_t layerGame;
};
