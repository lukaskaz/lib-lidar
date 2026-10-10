#include "scan/scan.hpp"

#include <cmath>

namespace Lidar
{

namespace Scan
{

Measurement Normal::getdata(bool firstread = false)
{
    static constexpr uint32_t packetsize = 5, poorquality = 10;
    std::vector<uint8_t> raw;

    auto recvsize =
        firstread ? serialIf->read(raw, 1, 2000) : serialIf->read(raw, 1);
    while (true)
    {
        bool flagbit = ((raw[0] >> 1) ^ raw[0]) & 0x01;
        if (flagbit)
        {
            recvsize += serialIf->read(raw, 1);
            bool syncbit = raw[1] & 0x01;
            if (syncbit)
            {
                size_t remainingsize = packetsize - recvsize;
                recvsize += serialIf->read(raw, remainingsize);
                if (recvsize != packetsize)
                {
                    throw std::runtime_error(
                        "Received packet size is incorrect");
                }

                [[maybe_unused]] bool newscan = raw[0] & 0x01;
                uint32_t quality = 100 * (raw[0] >> 2) / 63;

                int32_t angle{};
                double distance{};
                bool isvalid = quality > poorquality;
                if (isvalid)
                {
                    angle = static_cast<int32_t>(
                        lround(((raw[2] << 7) | (raw[1] >> 1)) / 64.));
                    distance = (((raw[4] << 8) | raw[3]) / 4.) / 10.;
                }
                return {isvalid, {angle, distance}};
            }
        }
        raw.clear();
        recvsize = serialIf->read(raw, 1);
    }
}

void Normal::processscan()
{
    auto [isvalid, data] = getdata(true);
    while (isrunning())
    {
        if (isvalid)
        {
            samplesmanager.update(data);
        }
        std::tie(isvalid, data) = getdata();
    }
}

void Normal::requestscan()
{
    serialIf->write({SCANSTARTFLAG, SCANSTARTSCAN});
    std::vector<uint8_t> resp;
    serialIf->read(resp, 7);
}

} // namespace Scan

} // namespace Lidar
