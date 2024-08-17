#pragma once

#include "interfaces/lidar.hpp"

#include <string>

class LidarFactory
{
  public:
    static std::shared_ptr<LidarIf> createAseries();
    static std::shared_ptr<LidarIf> createCseries();
};

class LidarFinder
{
  public:
    static std::shared_ptr<LidarIf> run(std::shared_ptr<serial>);
};
