#include "scan/scan.hpp"

#include <cmath>

namespace Lidar
{

namespace Scan
{

namespace Legacy
{

std::array<Measurement, 2> Express::getcabindata(std::vector<uint8_t>&& cabin,
                                                 double startangle,
                                                 double angledelta,
                                                 uint8_t cabinnum)
{
    auto calcangle = [startangle, angledelta](uint8_t anglecompq3,
                                              uint8_t measurement) {
        bool isnegative = anglecompq3 & 0x20 ? true : false;
        double anglecomp = (anglecompq3 & 0x1F) / 8.;
        anglecomp *= isnegative ? (-1) : 1;
        double angle =
            startangle + (angledelta * measurement / 32.) - anglecomp;
        return static_cast<int32_t>(
            lround(angle < MAXANGLEPERSCAN ? angle : angle - MAXANGLEPERSCAN));
    };

    int32_t anglefirst{};
    uint32_t distancefirst = ((cabin[0] >> 2) & 0x3F) | (cabin[1] << 6);

    bool isfirstvalid = distancefirst != 0;
    if (isfirstvalid)
    {
        uint8_t anglecompq3 =
                    (cabin[4] & 0x0F) | (uint8_t)((cabin[0] & 0x03) << 4),
                measurement = uint8_t(2 * cabinnum + 0);
        anglefirst = calcangle(anglecompq3, measurement + 1);
    }

    int32_t anglesecond{};
    uint32_t distancesecond = ((cabin[2] >> 2) & 0x3F) | (cabin[3] << 6);

    bool issecondvalid = distancesecond != 0;
    if (issecondvalid)
    {
        uint8_t anglecompq3 =
                    (uint8_t)(cabin[4] >> 4) | (((cabin[2] & 0x01)) & 0x0F),
                measurement = uint8_t(2 * cabinnum + 1);
        anglesecond = calcangle(anglecompq3, measurement + 1);
    }

    return {{{isfirstvalid, {anglefirst, distancefirst / 10.}},
             {issecondvalid, {anglesecond, distancesecond / 10.}}}};
}

void Express::processscan()
{
    [[maybe_unused]] bool newscan{false};
    auto [startangleprev, cabindataprev] = getbasedata(true);
    while (isrunning())
    {
        auto [startanglecurr, cabindatacurr] = getbasedata();
        auto angledelta = startanglecurr - startangleprev;
        if (angledelta < 0)
        {
            angledelta += MAXANGLEPERSCAN;
            newscan = true;
        }

        static constexpr uint8_t bytespercabin{5};
        for (auto it = cabindataprev.cbegin(); it != cabindataprev.cend();
             std::advance(it, bytespercabin))
        {
            auto cabinnum = static_cast<uint8_t>(
                std::distance(cabindataprev.cbegin(), it) / bytespercabin);
            auto measurements = getcabindata(
                {it, it + bytespercabin}, startangleprev, angledelta, cabinnum);

            std::ranges::for_each(measurements,
                                  [this](Measurement& measurement) {
                                      auto [isvalid, data] = measurement;
                                      if (isvalid)
                                          samplesmanager.update(data);
                                  });
        }
        startangleprev = startanglecurr;
        cabindataprev = cabindatacurr;
    }
}

} // namespace Legacy

} // namespace Scan

} // namespace Lidar
