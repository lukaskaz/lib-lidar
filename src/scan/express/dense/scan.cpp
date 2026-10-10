#include "scan/scan.hpp"

#include <cmath>

namespace Lidar
{

namespace Scan
{

namespace Dense
{

Measurement Express::getcabindata(std::vector<uint8_t>&& cabin,
                                  double startangle, double angledelta,
                                  uint8_t cabinnum)
{
    auto calcangle = [startangle, angledelta](uint8_t measurement) {
        double angle = startangle + (angledelta * measurement / 40.);
        return static_cast<uint32_t>(std::lround(
            angle < MAXANGLEPERSCAN ? angle : angle - MAXANGLEPERSCAN));
    };

    int32_t angle{};
    uint32_t distance = (cabin[0] & 0xFF) | (cabin[1] << 8);

    bool isvalid = distance != 0;
    if (isvalid)
    {
        angle = calcangle(cabinnum + 1);
    }

    return {isvalid, {angle, distance / 10.}};
}

void Express::processscan()
{
    [[maybe_unused]] bool newscan{false};
    auto [startangleprev, cabindataprev] = getbasedata(true);

    while (isrunning())
    {
        auto [startanglecurr, cabindatacurr] = getbasedata(false);
        auto angledelta = startanglecurr - startangleprev;
        if (angledelta <= 0)
        {
            angledelta += MAXANGLEPERSCAN;
            newscan = true;
        }

        static constexpr uint8_t bytespercabin{2};
        for (auto it = cabindataprev.cbegin(); it != cabindataprev.cend();
             std::advance(it, bytespercabin))
        {
            auto cabinnum = static_cast<uint8_t>(
                std::distance(cabindataprev.cbegin(), it) / bytespercabin);
            auto [isvalid, data] = getcabindata(
                {it, it + bytespercabin}, startangleprev, angledelta, cabinnum);
            if (isvalid)
            {
                samplesmanager.update(data);
            }
        }
        startangleprev = startanglecurr;
        cabindataprev = cabindatacurr;
    }
}

} // namespace Dense

} // namespace Scan

} // namespace Lidar
