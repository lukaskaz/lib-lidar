#include "lidarfactory.hpp"
#include "serial.hpp"

#include <iostream>

int main()
{
    try
    {
        const auto device{"/dev/ttyUSB0"};
        auto serial = std::make_shared<usb>(device);
        auto lidar = LidarFinder::run(serial);
        lidar->watchangle(
            0, Observer<SampleData>::create([](const SampleData& data) {
                if ((int)std::get<1>(data) < 30)
                {
                    std::cout << "CRITICAL: FRONT OBSTACLE TOO CLOSE\n";
                }
            }));
        lidar->watchangle(
            180, Observer<SampleData>::create([](const SampleData& data) {
                if ((int)std::get<1>(data) < 15)
                {
                    std::cout << " CRITICAL: REAR OBSTACLE TOO CLOSE\n";
                }
            }));
        lidar->runscan(scan_t::normal);
        usleep(100 * 1000);
        lidar->stopscan();
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }

    return 0;
}
