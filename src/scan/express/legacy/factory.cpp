#include "scan/factory.hpp"

#include "scan/scan.hpp"

namespace Lidar
{

namespace Scan
{

template <>
std::shared_ptr<Interface::Scan>
    Factory::create<scan_t::express, scansub_t::legacy>(
        std::shared_ptr<serial> serialIf)
{
    return std::shared_ptr<Legacy::Express>(new Legacy::Express(serialIf));
}

} // namespace Scan

} // namespace Lidar
