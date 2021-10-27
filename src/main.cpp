/**
 * GOALS
 * 1. Implement framework to allow testing many types of path-planning
 *    and navigation algorithms
 * 2. Play around with 2D Zelda-style terrain graphics
 * 3. Learn some modern C++ stuff along the way (C++17 / C++20)
  */

#include "main.hpp"

#include "pugixml/src/pugixml.hpp"

bool gamePaused = false;

bool AstarDemo::OnUserCreate()
{
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
    // Disable "clear-on-draw" so we only have to draw the background once
    EnableLayerClear(layerBG, false);

    SetDrawTarget(layerGame);
    Clear(olc::BLANK);

    DrawBackground();

    /** Setup the path-planning objects */
    astar.SetTerrainMap(gameMap);

    return true;
}

bool AstarDemo::OnUserUpdate(float fElapsedTime)
{
    (void)fElapsedTime; // Unused for now

    // Can uncomment in the future if using a dynamic background layer
    // DrawBackground();

    SetDrawTarget(layerGame);

    olc::vi2d mouse = GetMousePos();

    // Get the map tile at the mouse location, and its top-left coords
    // The current tile will always be used as the start location
    olc::vi2d tile_ij = {mouse.x / W, mouse.y / H};
    olc::vf2d tile_xy = {float(tile_ij.x * W), float(tile_ij.y * H)};

    // Highlight the map tile under the mouse
    SetPixelMode(olc::Pixel::ALPHA);
    DrawDecal(tile_xy, tileHighlight.Decal(), noscale, olc::WHITE);
    SetPixelMode(olc::Pixel::NORMAL);

    if (GetKey(olc::Key::P).bPressed) {
        gamePaused = !gamePaused;
    }

    if (!gamePaused) {

        bool newStart = false;
        if (tile_ij != startIJ) {
            newStart = true;
            startIJ = tile_ij;
        }

        // Set the 'goal tile' on any left-click
        // When clicking on the already-set goal tile, un-set it 
        bool newGoal = false;
        if (GetMouse(0).bPressed) {
            if (isGoalSet && goalIJ == tile_ij) {
                isGoalSet = false;

            } else {
                if (tile_ij != goalIJ) {
                    newGoal = true;
                }
                goalIJ = tile_ij;
                goalPos = tile_xy;
                isGoalSet = true;
            }
        }

        // If the goal tile has been set, display the shortest path
        
        if (isGoalSet && (newStart || newGoal)) {
            havePath = astar.ComputePath(tile_ij, goalIJ); 
        }
    }

    // If the goal tile has been set, display it
    if (isGoalSet) {
        SetPixelMode(olc::Pixel::ALPHA);
        DrawDecal(goalPos, tileHighlight.Decal(), noscale, olc::CYAN);
        SetPixelMode(olc::Pixel::NORMAL);
    }

    float cost = 0.f;
    if (havePath) {
        auto vPath = astar.GetPath();
        cost = astar.GetPathCost();

        if (vPath.size() > 0) {
            /// Draw the output from A*
            SetDrawTarget(layerGame);
            SetPixelMode(olc::Pixel::ALPHA);
            // Draw the returned path, skipping the start and goal tiles (already drawn)
            for (uint32_t i = 1; i < vPath.size() - 1; i++) {
                auto ij = vPath[i];
                olc::vf2d xy = {float(ij.x * W), float(ij.y * H)};
                DrawDecal(xy, tileHighlight.Decal(), noscale, olc::MAGENTA);
            }
            SetPixelMode(olc::Pixel::NORMAL);
        }
    }

    // Display the screen and map coordinates of the mouse
    std::stringstream ss;
    ss << "X: " << mouse.x << ", Y: " << mouse.y;
    ss << std::endl << std::endl;
    ss << "IX: " << tile_ij.x << ", IY: " << tile_ij.y;
    ss << std::endl << std::endl;
    ss << "Terrain Type: " << gameMap.GetTerrainAt(tile_ij.x, tile_ij.y); 
    ss << ", Effort: " << gameMap.GetEffortAt(tile_ij.x, tile_ij.y);
    ss << std::endl << std::endl;
    ss << "Path Cost:   " << cost;
    DrawStringDecal({5, ScreenHeight() - 62}, ss.str());

    if (gamePaused) {
        std::stringstream().swap(ss);
        ss << "PAUSED" << std::endl;
        DrawStringDecal({5, 5}, ss.str(), olc::WHITE, {2.f, 2.f});
    }

    return true;
}

void AstarDemo::DrawBackground()
{
    SetDrawTarget(layerBG);
    SetPixelMode(olc::Pixel::MASK);

    // Default background color
    FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::VERY_DARK_GREY);

    // Draw the world terrain map
    gameMap.Draw();

    // Draw Vertical Grid
    for (int i = 0; i < ScreenWidth() / W; i++) {
            DrawLine(i * W, 0, i * W, ScreenHeight(), olc::WHITE);
    }

    // Draw Horizontal Grid
    for (int i = 0; i < ScreenHeight() / H; i++) {
            DrawLine(0, i * H, ScreenWidth(), i * H, olc::WHITE);
    }
}

bool LoadInput(const std::string& fname, Config& config)
{
    std::ifstream f(fname, std::ifstream::in);

    if (!f.is_open()) {
        std::cout << "Unable to open: " << fname << std::endl;
        return false;
    }

    pugi::xml_document input;
    pugi::xml_parse_result res = input.load(f);
    if (res.status != pugi::status_ok) {
        std::cout << "Error parsing file " << fname << std::endl;
        return false;
    }

    config.fConfig = fname;
    config.dims.x = input.child("dims").attribute("x").as_int();
    config.dims.y = input.child("dims").attribute("y").as_int();
    config.sMap = input.child_value("map");
    std::cout << "dims: " << config.dims.x << ", " << config.dims.y << std::endl;
    std::cout << "map:" << config.sMap << std::endl;

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
    std::string fname("test.xml");
    if (argc > 1) {
        fname = argv[1];
    }

    Config config;

    if (!LoadInput(fname, config)) {
        print_usage(argv[0]);
        exit(1);
    }

    int32_t width = (config.dims.x - 2) * 32;
    int32_t height = (config.dims.y - 2) * 32;
    int32_t scale = 2;
    if (width * scale > 3840 || height * scale > 2160) {
        scale = 1;
    }

    AstarDemo demo(config);
    if (demo.Construct(width, height, scale, scale)) {
        demo.Start();
    }

    return 0;
}
