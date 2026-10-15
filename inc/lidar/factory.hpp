#pragma once

#include "lidar/enums.hpp"
#include "lidar/interface/lidar.hpp"

namespace Lidar
{

class Factory
{
  public:
    template <id_t Id>
    static std::shared_ptr<Interface::Lidar> create();
};

} // namespace Lidar
