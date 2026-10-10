#pragma once

#include "scan/interface/scan.hpp"
#include "serial.hpp"

#include <memory>

namespace Lidar
{

namespace Scan
{

class Factory
{
  public:
    template <scan_t Scan, scansub_t Scansub>
    static std::shared_ptr<Interface::Scan> create(std::shared_ptr<serial>);
};

} // namespace Scan

} // namespace Lidar
