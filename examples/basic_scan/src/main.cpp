#include "lidar/finder.hpp"
#include "serial.hpp"

#include <iostream>

using namespace Lidar;

int main()
{
    try
    {
        const auto device{"/dev/ttyUSB0"};
        auto serial = std::make_shared<usb>(device);
        auto lidar = Finder::run(serial);

        lidar->watchangle(
            0, Helpers::Observer<Samples::Data>::create(
                   [](const Samples::Data& data) {
                       if ((int)std::get<1>(data) < 30)
                       {
                           std::cout << "CRITICAL: FRONT OBSTACLE TOO CLOSE\n";
                       }
                   }));
        lidar->watchangle(
            180, Helpers::Observer<Samples::Data>::create(
                     [](const Samples::Data& data) {
                         if ((int)std::get<1>(data) < 15)
                         {
                             std::cout
                                 << " CRITICAL: REAR OBSTACLE TOO CLOSE\n";
                         }
                     }));
        lidar->runscan(Scan::scan_t::express);
        // usleep(100 * 1000);
        getchar();
        lidar->stopscan();
    }
    catch (std::exception& err)
    {
        std::cerr << "[ERROR] " << err.what() << '\n';
    }

    return 0;
}
