#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Lidar
{

struct Configuration
{
    struct Mode
    {
        uint32_t id;
        std::string name;
        uint32_t uscostpersample;
        uint32_t maxsamplerate;
        uint32_t maxdistance;
        uint32_t answercmdtype;
        bool operator==(const Mode&) const = default;
    };

    uint32_t modecnt;
    uint32_t typical;
    std::vector<Mode> modes;
    bool operator==(const Configuration&) const = default;
};

} // namespace Lidar
