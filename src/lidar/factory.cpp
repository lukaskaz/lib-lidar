#include "lidar/factory.hpp"

#include "lidar/lidar.hpp"
#include "scan/factory.hpp"

#include <algorithm>
#include <stdexcept>

namespace Lidar
{

std::shared_ptr<Interface::Lidar> Finder::run(std::shared_ptr<serial> serial)
{
    static const std::vector<std::shared_ptr<Interface::Lidar>> lidars = {
        Factory::create<seriesid::amodel>(),
        Factory::create<seriesid::cmodel>()};

    if (auto found = std::ranges::find_if(
            lidars, [&serial](auto lidar) { return lidar->setup(serial); });
        found != lidars.end())
    {
        return *found;
    }
    throw std::runtime_error("No registered lidar found");
}

} // namespace Lidar
