#include "helpers/helpers.hpp"

#include <algorithm>
#include <ctime>

namespace Lidar
{

namespace Helpers
{

std::string gettimestr()
{
    time_t rawtime{0};
    time(&rawtime);
    std::string timestr{asctime(localtime(&rawtime))};
    timestr.erase(std::remove(timestr.begin(), timestr.end(), '\n'),
                  timestr.end());
    return timestr;
}

uint8_t getchecksum(const std::vector<uint8_t>& data)
{
    uint8_t chsum{};
    std::ranges::for_each(data,
                          [&chsum](const uint8_t byte) { chsum ^= byte; });
    return chsum;
}

} // namespace Helpers

} // namespace Lidar
