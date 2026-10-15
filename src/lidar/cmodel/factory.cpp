#include "lidar/factory.hpp"

#include "lidar/lidar.hpp"
#include "scan/factory.hpp"

#include <termios.h>

#include <memory>

namespace Lidar
{

template <>
std::shared_ptr<Interface::Lidar>
    Factory::create<id_t{seriesid::cmodel, 1, 1}>()
{
    return std::shared_ptr<Lidar>(new Lidar(
        {seriesid::cmodel, 1, 1}, B460800,
        [](std::shared_ptr<serial> serial) -> scansinittype {
            return {Scan::Factory::create<Scan::scan_t::normal,
                                          Scan::scansub_t::none>(serial),
                    Scan::Factory::create<Scan::scan_t::express,
                                          Scan::scansub_t::dense>(serial)};
        }));
}

} // namespace Lidar
