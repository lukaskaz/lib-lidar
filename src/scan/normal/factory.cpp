#include "scan/factory.hpp"

#include "scan/scan.hpp"

namespace Lidar
{

namespace Scan
{

template <>
std::shared_ptr<Interface::Scan>
    Factory::create<scan_t::normal, scansub_t::none>(
        std::shared_ptr<serial> serialIf)
{
    return std::shared_ptr<Normal>(new Normal(serialIf));
}

} // namespace Scan

} // namespace Lidar
