#pragma once

#include "lidar/interface/lidar.hpp"
#include "serial.hpp"

#include <optional>

namespace Lidar
{

class Factory;

using scansinittype = std::vector<std::shared_ptr<Scan::Interface::Scan>>;
using scansinitfunc = std::function<scansinittype(std::shared_ptr<serial>)>;

class Lidar : public Interface::Lidar
{
  public:
    bool setup(std::shared_ptr<serial>) override;

    seriesid getseries() override;
    speed_t getspeed() override;
    std::string getname() override;
    std::tuple<std::string, std::string, std::string, std::string>
        getfwinfo() override;
    std::pair<state, std::string> getstate() override;
    std::pair<uint16_t, uint16_t> getsamplerate() override;
    Configuration getconfiguration() override;
    std::pair<std::string, std::string> getconninfo() override;

    void watchangle(int32_t,
                    std::shared_ptr<Helpers::Observer<Samples::Data>>) override;
    void runscan(Scan::scan_t) override;
    void stopscan() override;
    std::pair<std::string, std::string>
        getscaninfo(Scan::scan_t) const override;

  protected:
    const seriesid series;
    const speed_t baud;
    const scansinitfunc initscans;
    std::shared_ptr<serial> serialIf;
    std::vector<std::shared_ptr<Scan::Interface::Scan>> scans;
    uint8_t modelId{};
    std::optional<uint16_t> motorPwm;

  private:
    friend class Factory;
    Lidar(seriesid, speed_t, scansinitfunc&&);

    seriesid getseries(std::shared_ptr<serial>);
    void startmotor();
    void stopmotor();
    void getpacket(std::vector<uint8_t>&&, std::vector<uint8_t>&, uint8_t,
                   bool);
    std::shared_ptr<Scan::Interface::Scan> getscan(Scan::scan_t) const;
};

} // namespace Lidar
