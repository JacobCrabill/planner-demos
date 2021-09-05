/**
 * GOALS
 * 1. Implement framework to allow testing many types of path-planning
 *    and navigation algorithms
 * 2. Play around with 2D Zelda-style terrain graphics
 * 3. Learn some modern C++ stuff along the way (C++17 / C++20)
  */

#include "main.hpp"

bool AstarDemo::OnUserCreate()
{
    // Called once at the start, so create things here

    // Load the tile highlighter
    const std::string sHighlight = "resources/highlighted-square-1-32.png";
    auto ret = rHighlight.Load(sHighlight);

    if (ret != olc::rcode::OK) {
        std::cerr << "Error loading file: " << sHighlight << std::endl;
        exit(1);
    }

    tileMap.SetPGE(static_cast<PixelGameEngine*>(this));
    tileMap.LoadTerrainMap();

    // Clear the top layer so we can later draw to layers underneath
    SetPixelMode(olc::Pixel::MASK);
    SetDrawTarget(nullptr);
    Clear(olc::BLANK);

    layerGame = CreateLayer();
    EnableLayer(layerGame, true);

    layerBG = CreateLayer();
    EnableLayer(layerBG, true);
    // Disable "clear-on-draw" so we only have to draw the background once
    EnableLayerClear(layerBG, false);

    SetDrawTarget(layerGame);
    Clear(olc::BLANK);

    DrawBackground();

    /** Setup the path-planning objects */
    astar.SetTerrainMap(tileMap);

    return true;
}

bool AstarDemo::OnUserUpdate(float fElapsedTime)
{
    // Can uncomment in the future if using a dynamic background layer
    // DrawBackground();

    SetDrawTarget(layerGame);

    olc::vi2d mouse = GetMousePos();

    // Get the map tile at the mouse location, and its top-left coords
    // The current tile will always be used as the start location
    olc::vi2d tile_ij = {mouse.x / W, mouse.y / H};
    olc::vf2d tile_xy = {float(tile_ij.x * W), float(tile_ij.y * H)};

    bool newStart = false;
    if (tile_ij != vStartIJ) {
        newStart = true;
        vStartIJ = tile_ij;
    }

    // Set the 'goal tile' on any left-click
    // When clicking on the already-set goal tile, un-set it 
    bool newGoal = false;
    if (GetMouse(0).bPressed) {
        if (isGoalSet && vGoalIJ == tile_ij) {
            isGoalSet = false;

        } else {
            if (tile_ij != vGoalIJ) {
                newGoal = true;
            }
            vGoalIJ = tile_ij;
            vGoalPos = tile_xy;
            isGoalSet = true;
        }
    }

    // Highlight the map tile under the mouse
    SetPixelMode(olc::Pixel::ALPHA);
    DrawDecal(tile_xy, rHighlight.Decal(), noscale, olc::WHITE);
    SetPixelMode(olc::Pixel::NORMAL);

    // If the goal tile has been set, display it
    if (isGoalSet) {
        SetPixelMode(olc::Pixel::ALPHA);
        DrawDecal(vGoalPos, rHighlight.Decal(), noscale, olc::CYAN);
        SetPixelMode(olc::Pixel::NORMAL);
    }

    // If the goal tile has been set, display the shortest path
    
    if (isGoalSet && (newStart || newGoal)) {
        havePath = astar.ComputePath(tile_ij, vGoalIJ); 
    }

    float cost = 0.f;
    if (havePath) {
        auto vPath = astar.GetPath();
        cost = astar.GetPathCost();

        /// Draw the output from A*
        SetDrawTarget(layerGame);
        SetPixelMode(olc::Pixel::ALPHA);
        for (auto& ij : vPath) {
            olc::vf2d xy = {float(ij.x * W), float(ij.y * H)};
            DrawDecal(xy, rHighlight.Decal(), noscale, olc::MAGENTA);
        }
        SetPixelMode(olc::Pixel::NORMAL);
    }

    // Display the screen and map coordinates of the mouse
    std::stringstream ss;
    ss << "X: " << mouse.x << ", Y: " << mouse.y;
    ss << std::endl << std::endl;
    ss << "IX: " << tile_ij.x << ", IY: " << tile_ij.y;
    ss << std::endl << std::endl;
    ss << "Path Cost: " << cost;
    std::string status = ss.str();
    DrawStringDecal({5, 5}, status);


    return true;
}

void AstarDemo::DrawBackground()
{
    SetDrawTarget(layerBG);
    SetPixelMode(olc::Pixel::MASK);

    // Default background color
    FillRect(0, 0, WIDTH, HEIGHT, olc::VERY_DARK_GREY);

    // Draw the world terrain map
    tileMap.Draw();

    // Draw Vertical Grid
    for (int i = 0; i < WIDTH / W; i++) {
            DrawLine(i * W, 0, i * W, HEIGHT, olc::WHITE);
    }

    // Draw Horizontal Grid
    for (int i = 0; i < HEIGHT / H; i++) {
            DrawLine(0, i * H, WIDTH, i * H, olc::WHITE);
    }
}

int main()
{
    AstarDemo demo;
    if (demo.Construct(WIDTH, HEIGHT, 2, 2)) {
        demo.Start();
    }

    return 0;
}
