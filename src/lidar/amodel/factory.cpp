#include "lidar/factory.hpp"

#include "lidar/lidar.hpp"
#include "scan/factory.hpp"

#include <termios.h>

#include <memory>

namespace Lidar
{

template <>
std::shared_ptr<Interface::Lidar>
    Factory::create<id_t{seriesid::amodel, 1, 8}>()
{
    return std::shared_ptr<Lidar>(new Lidar(
        {seriesid::amodel, 1, 8}, B115200,
        [](std::shared_ptr<serial> serial) -> scansinittype {
            return {Scan::Factory::create<Scan::scan_t::normal,
                                          Scan::scansub_t::none>(serial),
                    Scan::Factory::create<Scan::scan_t::express,
                                          Scan::scansub_t::legacy>(serial)};
        }));
}

template <>
std::shared_ptr<Interface::Lidar>
    Factory::create<id_t{seriesid::amodel, 2, 12}>()
{
    return std::shared_ptr<Lidar>(new Lidar(
        {seriesid::amodel, 2, 12}, B256000,
        [](std::shared_ptr<serial> serial) -> scansinittype {
            return {Scan::Factory::create<Scan::scan_t::normal,
                                          Scan::scansub_t::none>(serial),
                    Scan::Factory::create<Scan::scan_t::express,
                                          Scan::scansub_t::legacy>(serial)};
        }));
}

} // namespace Lidar
