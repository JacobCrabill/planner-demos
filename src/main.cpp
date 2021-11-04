/**
 * GOALS
 * 1. Implement framework to allow testing many types of path-planning
 *    and navigation algorithms
 * 2. Play around with 2D Zelda-style terrain graphics
 * 3. Learn some modern C++ stuff along the way (C++17 / C++20)
  */

#include "main.hpp"

#include "yaml-cpp/yaml.h"

bool gamePaused = false;

bool AstarDemo::OnUserCreate()
{
    PROFILE_FUNC();

    // Called once at the start, so create things here

    // Load the tile highlighter
    const std::string fname = "resources/highlighted-square-1-32.png";
    auto ret = tileHighlight.Load(fname);

    if (ret != olc::rcode::OK) {
        std::cerr << "Error loading file: " << fname << std::endl;
        exit(1);
    }

    gameMap.SetPGE(static_cast<PixelGameEngine*>(this));
    gameMap.GenerateMap();

    // Clear the top layer so we can later draw to layers underneath
    SetPixelMode(olc::Pixel::MASK);
    SetDrawTarget(nullptr);
    Clear(olc::BLANK);

    layerGame = (uint8_t)CreateLayer();
    EnableLayer(layerGame, true);

    layerBG = (uint8_t)CreateLayer();
    EnableLayer(layerBG, true);
    // Note: We should consider disabling "clear-on-draw" when not updating the background
    // so we only have to draw the background when we need to
    EnableLayerClear(layerBG, true);

    SetDrawTarget(layerGame);
    Clear(olc::BLANK);

    DrawBackground();

    switch (config.method) {
        case ASTAR:
            planner = new AStar();
            break;

        case RRTSTAR:
        default:
            std::cout << "WARNING: Unrecognized planner method requested. Defaulting to A*." << std::endl;
            planner = new AStar();
            break;
    }

    /** Setup the path-planning objects */
    planner->SetTerrainMap(gameMap);

    return true;
}

bool AstarDemo::OnUserUpdate(float fElapsedTime)
{
    PROFILE_FUNC();

    UpdateCursor();

    GetUserInput();

    const float panSpeed = 250.f;
    if (wPressed) viewOffset.y -= fElapsedTime * panSpeed;
    if (aPressed) viewOffset.x -= fElapsedTime * panSpeed;
    if (sPressed) viewOffset.y += fElapsedTime * panSpeed;
    if (dPressed) viewOffset.x += fElapsedTime * panSpeed;

    viewOffset.x = std::min(std::max(0.f, viewOffset.x), (float)((config.dims.x - 2) * 32 - ScreenWidth()));
    viewOffset.y = std::min(std::max(0.f, viewOffset.y), (float)((config.dims.y - 2) * 32 - ScreenHeight()));

    DrawBackground();

    SetDrawTarget(layerGame);

    // Highlight the map tile under the mouse
    SetPixelMode(olc::Pixel::ALPHA);
    DrawDecal(mTileXY - viewOffset, tileHighlight.Decal(), noscale, olc::WHITE);
    SetPixelMode(olc::Pixel::NORMAL);

    if (!gamePaused) {

        bool newStart = false;
        if (mTileIJ != startIJ) {
            newStart = true;
            startIJ = mTileIJ;
        }

        // Set the 'goal tile' on any left-click
        // When clicking on the already-set goal tile, un-set it
        bool newGoal = false;
        if (GetMouse(0).bPressed) {
            if (isGoalSet && goalIJ == mTileIJ) {
                isGoalSet = false;

            } else {
                if (mTileIJ != goalIJ) {
                    newGoal = true;
                }
                goalIJ = mTileIJ;
                goalPos = mTileXY;
                isGoalSet = true;
            }
        }

        // If the goal tile has been set, display the shortest path
        
        if (isGoalSet && (newStart || newGoal)) {
            havePath = planner->ComputePath(mTileIJ, goalIJ);
            pathCost = planner->GetPathCost();
        }
    }

    // If the goal tile has been set, display it
    if (isGoalSet) {
        SetPixelMode(olc::Pixel::ALPHA);
        DrawDecal(goalPos - viewOffset, tileHighlight.Decal(), noscale, olc::CYAN);
        SetPixelMode(olc::Pixel::NORMAL);
    }

    DrawPath();

    PrintOverlay();

    return true;
}

void AstarDemo::DrawBackground()
{
    PROFILE_FUNC();

    SetDrawTarget(layerBG);
    SetPixelMode(olc::Pixel::MASK);

    // Draw the world terrain map
    gameMap.Draw({(int)viewOffset.x, (int)viewOffset.y});
}

void AstarDemo::GetUserInput()
{
    /**
     * User Input / Keyboard Controls:
     * - WASD to pan the map
     * - C to recenter the map
     * - P to pause the pathfinding
     */

    if (GetKey(olc::Key::W).bPressed) {
        wPressed = true;
    } else if (GetKey(olc::Key::W).bReleased) {
        wPressed = false;
    }

    if (GetKey(olc::Key::A).bPressed) {
        aPressed = true;
    } else if (GetKey(olc::Key::A).bReleased) {
        aPressed = false;
    }

    if (GetKey(olc::Key::S).bPressed) {
        sPressed = true;
    } else if (GetKey(olc::Key::S).bReleased) {
        sPressed = false;
    }

    if (GetKey(olc::Key::D).bPressed) {
        dPressed = true;
    } else if (GetKey(olc::Key::D).bReleased) {
        dPressed = false;
    }

    if (GetKey(olc::Key::C).bPressed) {
        viewOffset = {0.f, 0.f};
    }

    if (GetKey(olc::Key::P).bPressed) {
        gamePaused = !gamePaused;
    }
}

void AstarDemo::UpdateCursor()
{
    mouse = GetMousePos();
    wMouse = mouse + viewOffset;

    olc::vf2d tileOffset = {fmodf(viewOffset.x, (float)TW), fmodf(viewOffset.y, (float)TH)};

    // Get the map tile at the mouse location, and its top-left coords
    // The current tile will always be used as the start location
    mTileIJ = {(int)wMouse.x / TW, (int)wMouse.y / TH};
    mTileXY = {mTileIJ.x * TW, mTileIJ.y * TH};
}

void AstarDemo::DrawPath()
{
    if (havePath) {
        auto vPath = planner->GetPath();

        if (vPath.size() > 0) {
            /// Draw the output from A*
            SetDrawTarget(layerGame);
            SetPixelMode(olc::Pixel::ALPHA);
            // Draw the returned path, skipping the start and goal tiles (already drawn)
            for (uint32_t i = 1; i < vPath.size() - 1; i++) {
                auto ij = vPath[i];
                olc::vf2d xy = {float(ij.x * TW), float(ij.y * TH)};
                xy -= viewOffset;
                DrawDecal(xy, tileHighlight.Decal(), noscale, olc::MAGENTA);
            }
            SetPixelMode(olc::Pixel::NORMAL);
        }
    }
}

void AstarDemo::PrintOverlay()
{
    PROFILE_FUNC();

    // USEFUL NOTE: The default character size is (8px by 8px) * (scale value)

    // Display the screen/map coordinates of the mouse / tile, plus path / terrain stuff
    // This one goes in the lower-left of the screen currently
    std::stringstream ss;
    ss << "Screen X: " << mouse.x << ", Screen Y: " << mouse.y;
    ss << std::endl << std::endl;
    ss << "World X: " << wMouse.x << ", World Y: " << wMouse.y;
    ss << std::endl << std::endl;
    ss << "IX: " << mTileIJ.x << ", IY: " << mTileIJ.y;
    ss << std::endl << std::endl;
    ss << "Terrain Type: " << gameMap.GetTerrainAt(mTileIJ.x, mTileIJ.y);
    ss << ", Effort: " << gameMap.GetEffortAt(mTileIJ.x, mTileIJ.y);
    ss << std::endl << std::endl;
    ss << "Path Cost:   " << pathCost;
    DrawStringDecal({5, ScreenHeight() - 9*8-4}, ss.str());

    // Second status in top-left: PAUSED indicator + keys pressed
    if (gamePaused) {
        std::stringstream().swap(ss);
        ss << "PAUSED" << std::endl;
        DrawStringDecal({5, 5}, ss.str(), olc::WHITE, {2.f, 2.f});
    }

    std::stringstream().swap(ss);
    if (wPressed) ss << "W";
    if (aPressed) ss << "A";
    if (sPressed) ss << "S";
    if (dPressed) ss << "D";
    ss << std::endl;
    DrawStringDecal({5, 5 + (int)gamePaused*16.f}, ss.str(), olc::RED, {2.f, 2.f});
}

bool LoadInput(const std::string& fname, Config& config)
{
    YAML::Node input = YAML::LoadFile(fname);

    config.fConfig = fname;
    config.dims.x = input["dims"]["x"].as<int>();
    config.dims.y = input["dims"]["y"].as<int>();
    config.mapType = MapTypeValFromString(input["maptype"].as<std::string>());
    config.method = MethodValFromString(input["method"].as<std::string>());
    
    // Optional configuration
    if (config.mapType == MapType::STATIC) {
        if (input["map"]) {
            config.map = input["map"].as<std::vector<int>>();
        } else {
            std::cout << "Static map requested but map not given" << std::endl;
            exit(1);
        }
    }
    
    if (input["seed"]) {
        config.seed = input["seed"].as<int>();
    }

    return true;
}

void print_usage(const std::string& arg0)
{
    std::cout << "Invalid input file" << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "    " << arg0 << " <input_config>" << std::endl;
}

int main(int argc, char* argv[])
{
    std::string fname("test-procedural.yaml");
    if (argc > 1) {
        fname = argv[1];
    }

    Config config;

    if (!LoadInput(fname, config)) {
        print_usage(argv[0]);
        exit(1);
    }

    const int width = std::min(1024, (config.dims.x - 2) * 32);
    const int height = std::min(768, (config.dims.y - 2) * 32);

#ifdef INCLUDE_PROFILING
    Instrumentor::Get().BeginSession("planner-demo", "results.json");
#endif

    AstarDemo demo(config);
    if (demo.Construct(width, height, 2, 2)) {
        demo.Start();
    }

#ifdef INCLUDE_PROFILING
    Instrumentor::Get().EndSession();
#endif

    return 0;
}
