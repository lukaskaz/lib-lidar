#pragma once

#include "lidar/interface/lidar.hpp"

namespace Lidar
{

class Finder
{
  public:
    static std::shared_ptr<Interface::Lidar> run(std::shared_ptr<serial>);
};

} // namespace Lidar
