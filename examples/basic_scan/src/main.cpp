
#include "lidarfactory.hpp"

#include <iostream>

int main()
{
    try
    {
        const auto device{"/dev/ttyUSB0"};
        auto lidar = LidarFinder::run(device);
        lidar->watchangle(0, [](const SampleData& data) {
            if ((int)std::get<1>(data) < 30)
            {
                std::cout << "CRITICAL: FRONT OBSTACLE TOO CLOSE\n";
            }
        });
        lidar->watchangle(180, [](const SampleData& data) {
            if ((int)std::get<1>(data) < 15)
            {
                std::cout << " CRITICAL: REAR OBSTACLE TOO CLOSE\n";
            }
        });
        lidar->runscan(scan_t::express);
        usleep(100 * 1000);
        lidar->stopscan();
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }

    return 0;
}
