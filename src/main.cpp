/**
 * GOALS
 * 1. Implement framework to allow testing many types of path-planning
 *    and navigation algorithms
 * 2. Play around with 2D Zelda-style terrain graphics
 * 3. Learn some modern C++ stuff along the way (C++17 / C++20)
  */
#define OLC_PGE_APPLICATION
#include "plannerDemo.hpp"

void print_usage(const std::string& arg0)
{
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

    PlannerDemo demo(config);
    if (demo.Construct(width, height, 2, 2)) {
        demo.Start();
    }

#ifdef INCLUDE_PROFILING
    Instrumentor::Get().EndSession();
#endif

    return 0;
}
