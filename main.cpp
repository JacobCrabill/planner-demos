/**
 * GOALS
 * 1. Implement framework to allow testing many types of path-planning and navigation algorithms
  */
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

#define WIDTH 640
#define HEIGHT 480

#define W 32
#define H 32

#include "tilemap.hpp"
#include "astar.hpp"

// Override base class with your custom functionality
class AstarDemo : public olc::PixelGameEngine
{
    public:
        AstarDemo()
        {
            // Name you application
            sAppName = "AstarDemo";
        }

        olc::Renderable rHighlight;

        olc::vi2d vGoalIJ;
        olc::vf2d vGoalPos;
        bool isGoalSet {false};

        const olc::vf2d noscale = {1.f, 1.f};

        std::shared_ptr<AStar> astar {nullptr};
        std::shared_ptr<Tile> tile1 {nullptr};
        TileMap tileMap;

        uint8_t layerBG;
        uint8_t layerGame;

        bool BackgroundDrawn {false};

    public:
        bool OnUserCreate() override
        {
            // Called once at the start, so create things here

            // Load the tile highlighter
            const std::string sHighlight = "resources/highlighted-square-1-32.png";
            auto ret = rHighlight.Load(sHighlight);

            if (ret != olc::rcode::OK) {
                std::cerr << "Error loading file: " << sHighlight << std::endl;
                exit(1);
            }

            tile1 = std::make_shared<Tile>(this, rHighlight, olc::vf2d({32*5.f, 32*5.f}), .1f);
            tileMap.AddTile(tile1);

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

            return true;
        }

        bool OnUserUpdate(float fElapsedTime) override
        {
            // Can uncomment in the future if using a dynamic background layer
            // DrawBackground();

            SetDrawTarget(layerGame);

            olc::vi2d mouse = GetMousePos();

            // Get the map tile at the mouse location, and its top-left coords
            int32_t tile_i = mouse.x / W;
            int32_t tile_j = mouse.y / H;
            float tile_x = static_cast<float>(tile_i * W);
            float tile_y = static_cast<float>(tile_j * H);

            // Set the 'goal tile' on any left-click
            // When clicking on the already-set goal tile, un-set it 
            if (GetMouse(0).bPressed) {
                if (isGoalSet && vGoalIJ == olc::vi2d({tile_i, tile_j})) {
                    isGoalSet = false;

                } else {
                    vGoalIJ = {tile_i, tile_j};
                    vGoalPos = {tile_x, tile_y};
                    isGoalSet = true;
                }
            }

            // Display the screen and map coordinates of the mouse
            std::stringstream ss;
            ss << "X: " << mouse.x << ", Y: " << mouse.y;
            ss << std::endl << std::endl;
            ss << "IX: " << tile_i << ", IY: " << tile_y;
            std::string status = ss.str();
            DrawStringDecal({5, 5}, status);

            // Highlight the map tile under the mouse
            SetPixelMode(olc::Pixel::ALPHA);
            DrawDecal({tile_x, tile_y}, rHighlight.Decal(), noscale, olc::WHITE);
            SetPixelMode(olc::Pixel::NORMAL);

            // If the goal tile has been set, display it
            if (isGoalSet) {
                SetPixelMode(olc::Pixel::ALPHA);
                DrawDecal(vGoalPos, rHighlight.Decal(), noscale, olc::CYAN);
                SetPixelMode(olc::Pixel::NORMAL);
            }

            //DrawDecal(tile1->GetPos(), tile1->GetDecal());
            //tile1->Draw();
            tileMap.Draw();

            return true;
        }

        void DrawBackground()
        {
            SetDrawTarget(layerBG);
            SetPixelMode(olc::Pixel::MASK);
            FillRect(0, 0, WIDTH, HEIGHT, olc::VERY_DARK_GREY);

            // Draw Vertical Grid
            for (int i = 0; i < WIDTH / 32; i++) {
                    DrawLine(i * 32, 0, i * 32, HEIGHT, olc::WHITE);
            }

            // Draw Horizontal Grid
            for (int i = 0; i < HEIGHT / 32; i++) {
                    DrawLine(0, i * 32, WIDTH, i * 32, olc::WHITE);
            }
        }
};

int main()
{
    AstarDemo demo;
    if (demo.Construct(WIDTH, HEIGHT, 2, 2)) {
        demo.Start();
    }

    return 0;
}
