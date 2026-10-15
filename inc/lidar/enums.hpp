#pragma once

#include <cstdint>
#include <string>

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

struct id_t
{
    seriesid series;
    uint8_t model;
    uint8_t submodel;

    bool operator==(const id_t&) const = default;
};

} // namespace Lidar
