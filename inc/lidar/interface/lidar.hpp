#pragma once

#include "helpers/helpers.hpp"
#include "lidar/config.hpp"
#include "lidar/enums.hpp"
#include "scan/interface/scan.hpp"
#include "serial.hpp"

#include <memory>

namespace Lidar
{

namespace Interface
{

class LidarInfo
{
  public:
    virtual ~LidarInfo()
    {}

    virtual id_t getid() = 0;
    virtual speed_t getspeed() = 0;
    virtual std::string getname() = 0;
    virtual std::tuple<std::string, std::string, std::string, std::string>
        getfwinfo() = 0;
    virtual std::pair<state, std::string> getstate() = 0;
    virtual std::pair<uint16_t, uint16_t> getsamplerate() = 0;
    virtual Configuration getconfiguration() = 0;
    virtual std::pair<std::string, std::string> getconninfo() = 0;
};

class LidarScan
{
  public:
    virtual ~LidarScan()
    {}

    virtual void
        watchangle(int32_t,
                   std::shared_ptr<Helpers::Observer<Samples::Data>>) = 0;
    virtual void runscan(Scan::scan_t) = 0;
    virtual void stopscan() = 0;
    virtual std::pair<std::string, std::string>
        getscaninfo(Scan::scan_t) const = 0;
};

class Lidar : public LidarInfo, public LidarScan
{
  public:
    virtual ~Lidar()
    {}

    virtual bool setup(std::shared_ptr<serial>) = 0;
};

} // namespace Interface

} // namespace Lidar
