#pragma once

namespace Lidar
{

enum class state
{
    good,
    warn,
    err
};

enum class seriesid
{
    amodel = 0,
    cmodel = 4,
    smodel = 6,
    tmodel = 9,
    mmodel = 12,
    unknown = 255
};

} // namespace Lidar
