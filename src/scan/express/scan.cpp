#include "scan/scan.hpp"

namespace Lidar
{

namespace Scan
{

void Base::Express::requestscan()
{
    serialIf->write(
        {SCANSTARTFLAG, 0x82, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22});
    std::vector<uint8_t> resp;
    serialIf->read(resp, 7);
}

std::pair<double, std::vector<uint8_t>>
    Base::Express::getbasedata(bool firstread)
{
    static constexpr uint32_t packetsize = 84;
    std::vector<uint8_t> raw;

    auto recvsize =
        firstread ? serialIf->read(raw, 1, 2000) : serialIf->read(raw, 1);
    while (true)
    {
        uint8_t sync1 = (raw[0] & 0xF0) >> 4;
        if (sync1 == 0x0A)
        {
            recvsize += serialIf->read(raw, 1);
            uint8_t sync2 = (raw[1] & 0xF0) >> 4;
            if (sync2 == 0x05)
            {
                recvsize += serialIf->read(raw, 2);
                bool startflag = raw[3] & 0x80 ? true : false;
                if (firstread && !startflag)
                {
                    throw std::runtime_error("First packet without start flag");
                }

                size_t remainingsize = packetsize - recvsize;
                recvsize += serialIf->read(raw, remainingsize);
                if (recvsize != packetsize)
                {
                    throw std::runtime_error(
                        "Received packet size is incorrect");
                }

                uint8_t chsumrecv =
                    (uint8_t)((raw[1] & 0x0F) << 4) | (raw[0] & 0x0F);
                uint8_t chsumcalc =
                    Helpers::getchecksum({raw.begin() + 2, raw.end()});
                if (chsumrecv == chsumcalc)
                {
                    double startangle =
                        static_cast<double>(((raw[3] & 0x7F) << 8) | raw[2]) /
                        64.;
                    std::vector<uint8_t> cabindata{raw.begin() + 4, raw.end()};
                    return {startangle, std::move(cabindata)};
                }
            }
        }
        raw.clear();
        recvsize = serialIf->read(raw, 1);
    }
}

} // namespace Scan

} // namespace Lidar
