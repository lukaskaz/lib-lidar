#include "lidar/finder.hpp"

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
        Factory::create<id_t{seriesid::amodel, 1, 8}>(),
        Factory::create<id_t{seriesid::amodel, 2, 12}>(),
        Factory::create<id_t{seriesid::cmodel, 1, 1}>()};

    if (auto found = std::ranges::find_if(
            lidars, [&serial](auto lidar) { return lidar->setup(serial); });
        found != lidars.end())
    {
        return *found;
    }
    throw std::runtime_error("No registered lidar found");
}

} // namespace Lidar
