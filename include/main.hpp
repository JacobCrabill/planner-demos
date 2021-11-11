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

private:
    void GetUserInput();

    void UpdateCursor();
    
    void DrawPath();

    void DrawBackground();

    void PrintOverlay();

    olc::Renderable tileHighlight;

    // Cursor Location
    olc::vi2d mouse;   //!< Screen position of mouse
    olc::vf2d wMouse;  //!< World position of mouse
    olc::vi2d mTileIJ; //!< World I,J coordinates
    olc::vi2d mTileXY; //!< World X,Y coordinates

    // Screen motion with WASD
    olc::vf2d viewOffset {0.f, 0.f};
    bool wPressed {false};
    bool aPressed {false};
    bool sPressed {false};
    bool dPressed {false};

    // Planner Variables
    olc::vi2d goalIJ;
    olc::vi2d startIJ;
    olc::vf2d goalPos;
    float pathCost {0.f};
    bool isGoalSet {false};
    bool havePath {false};
    bool gamePaused {false};

    const olc::vf2d noscale = {1.f, 1.f};

    Planner* planner;
    GameMap gameMap;
    Config config;

    uint8_t layerBG;
    uint8_t layerGame;
};
