#pragma once

#include "lidar/interface/lidar.hpp"

namespace Lidar
{

class Factory
{
  public:
    template <seriesid Model>
    static std::shared_ptr<Interface::Lidar> create();
};

} // namespace Lidar
