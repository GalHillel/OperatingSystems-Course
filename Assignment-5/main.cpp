#include <iostream>
#include <cstdlib>
#include <ctime>
#include "StPipeline.hpp"

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3)
    {
        std::cout << "Usage: ./st_pipeline N [seed]" << std::endl;
        return 1;
    }

    unsigned int N = std::atoi(argv[1]);
    unsigned int seed = (argc == 3) ? std::atoi(argv[2]) : static_cast<unsigned int>(std::time(nullptr));

    StPipeline pipeline(N, seed);
    pipeline.run();

    return 0;
}
