#include "lidar/lidar.hpp"

#include "helpers/helpers.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_map>

namespace Lidar
{

constexpr auto SCANSTARTFLAG = 0xA5;
constexpr auto SCANGETINFOCMD = 0x50;
constexpr auto SCANGETSTATCMD = 0x52;
constexpr auto SCANGETSRATECMD = 0x59;
constexpr auto SCANGETCONFCMD = 0x84;

bool Lidar::setup(std::shared_ptr<serial> tmpserial)
{
    tmpserial->setBaud(getspeed());
    if (series == getseries(tmpserial))
    {
        serialIf = tmpserial;
        scans = initscans(serialIf);
        return true;
    }
    return false;
}

seriesid Lidar::getseries()
{
    return series;
}

speed_t Lidar::getspeed()
{
    return baud;
}

std::string Lidar::getname()
{
    std::string name;
    serialIf->write({SCANSTARTFLAG, SCANGETINFOCMD});
    std::vector<uint8_t> resp;
    if (serialIf->read(resp, 27))
    {
        uint8_t modelid = resp[7];
        uint8_t majormodelid = (modelid >> 4) & 0x0F;
        uint8_t submodelid = modelid & 0x0F;
        auto getname = [majormodelid, submodelid](char prefix, uint8_t offset) {
            return prefix + std::to_string(majormodelid - offset) + "M" +
                   std::to_string(submodelid);
        };

        if (majormodelid >= (uint8_t)seriesid::mmodel)
        {
            name = getname('M', (uint8_t)seriesid::mmodel - 1);
        }
        else if (majormodelid >= (uint8_t)seriesid::tmodel)
        {
            name = getname('T', (uint8_t)seriesid::tmodel - 1);
        }
        else if (majormodelid >= (uint8_t)seriesid::smodel)
        {
            name = getname('S', (uint8_t)seriesid::smodel - 1);
        }
        else if (majormodelid >= (uint8_t)seriesid::cmodel)
        {
            name = getname('C', (uint8_t)seriesid::cmodel - 1);
        }
        else
        {
            name = getname('A', 0);
        }
    }
    return name;
}

void Lidar::watchangle(int32_t angle,
                       std::shared_ptr<Helpers::Observer<Samples::Data>> obs)
{
    getscan(Scan::scan_t::normal)->addangle(angle, obs);
    getscan(Scan::scan_t::express)->addangle(angle, obs);
}

std::tuple<std::string, std::string, std::string, std::string>
    Lidar::getfwinfo()
{
    serialIf->write({SCANSTARTFLAG, SCANGETINFOCMD});
    std::vector<uint8_t> resp;
    serialIf->read(resp, 27);

    std::ostringstream oss;
    oss << std::showbase << std::hex << (uint32_t)resp[7];
    std::string model{oss.str()};
    oss.str(std::string{});
    oss << std::dec << (uint32_t)resp[9] << "." << (uint32_t)resp[8];
    std::string firmware{oss.str()};
    oss.str(std::string{});
    oss << std::hex << std::showbase << (uint32_t)resp[10];
    std::string hardware{oss.str()};
    oss.str(std::string{});
    for (uint8_t byte = 11; byte < 27; byte++)
    {
        oss << std::hex << std::noshowbase << (uint32_t)resp[byte];
    }
    std::string serialnum{oss.str()};
    return {model, firmware, hardware, serialnum};
}

std::pair<state, std::string> Lidar::getstate()
{
    static const std::unordered_map<state, std::string> status = {
        {state::good, "\e[1;32mOk\e[0m"},
        {state::warn, "\e[1;33mWarning\e[0m"},
        {state::err, "\e[1;31mERROR\e[0m"}};

    serialIf->write({SCANSTARTFLAG, SCANGETSTATCMD});
    std::vector<uint8_t> resp;
    serialIf->read(resp, 10);

    auto code = static_cast<state>(resp.at(7));
    auto name = status.contains(code) ? status.at(code) : std::string{};
    return {code, name};
}

std::pair<uint16_t, uint16_t> Lidar::getsamplerate()
{
    static constexpr auto makeval = [](uint8_t hi, uint8_t low) -> uint16_t {
        return hi << 8 | low;
    };
    serialIf->write({SCANSTARTFLAG, SCANGETSRATECMD});
    std::vector<uint8_t> resp;
    serialIf->read(resp, 11);
    auto normalscantime{makeval(resp[8], resp[7])};
    auto expressscantime{makeval(resp[10], resp[9])};
    return {normalscantime, expressscantime};
}

Configuration Lidar::getconfiguration()
{
    static constexpr uint8_t reqdatasize = 4, respdatasize = reqdatasize,
                             resppacketsize = respdatasize + 7;
    static constexpr auto removenull = [](auto& vect) {
        if (!vect.empty() && vect.back() == 0x00)
            vect.pop_back();
    };
    Configuration config;
    std::vector<uint8_t> resp;

    getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize, 0x70, 0, 0, 0}, resp,
              resppacketsize + 2, true);
    config.modecnt = resp[12] << 8 | resp[11];

    resp.clear();
    getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize, 0x7C, 0, 0, 0}, resp,
              resppacketsize + 2, true);
    config.typical = resp[12] << 8 | resp[11];

    for (uint8_t mode = 0; mode < config.modecnt; mode++)
    {
        resp.clear();
        getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize + 2, 0x7F, 0, 0,
                   0, mode, 0},
                  resp, resppacketsize + 200, true);
        removenull(resp);
        std::string name(resp.begin() + 11, resp.end());

        resp.clear();
        getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize + 2, 0x71, 0, 0,
                   0, mode, 0},
                  resp, resppacketsize + 4, true);
        uint32_t uscostpersample =
            resp[14] << 24 | resp[13] << 16 | resp[12] << 8 | resp[11];
        uscostpersample /= 256;
        uint32_t maxsamplerate = 1000 * 1000 / uscostpersample;

        resp.clear();
        getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize + 2, 0x74, 0, 0,
                   0, mode, 0},
                  resp, resppacketsize + 4, true);
        uint32_t maxdistance =
            resp[14] << 24 | resp[13] << 16 | resp[12] << 8 | resp[11];
        maxdistance /= 256;

        resp.clear();
        getpacket({SCANSTARTFLAG, SCANGETCONFCMD, reqdatasize + 2, 0x75, 0, 0,
                   0, mode, 0},
                  resp, resppacketsize + 1, true);
        uint32_t answercmdtype = resp[11];

        config.modes.emplace_back(mode, name, uscostpersample, maxsamplerate,
                                  maxdistance, answercmdtype);
    }

    return config;
}

std::pair<std::string, std::string> Lidar::getconninfo()
{
    return {serialIf->getdevice(), serialIf->getbaud()};
}

void Lidar::runscan(Scan::scan_t type)
{
    const auto scan = getscan(type);
    if (series == seriesid::amodel)
    {
        startmotor();
    }
    scan->run();
}

void Lidar::stopscan()
{
    std::exception_ptr stopError;
    for (const auto& scan : scans)
    {
        if (scan->isrunning())
        {
            try
            {
                scan->stop();
            }
            catch (...)
            {
                if (!stopError)
                {
                    stopError = std::current_exception();
                }
            }
        }
    }
    if (series == seriesid::amodel)
    {
        try
        {
            stopmotor();
        }
        catch (...)
        {
            if (!stopError)
            {
                stopError = std::current_exception();
            }
        }
    }
    if (stopError)
    {
        std::rethrow_exception(stopError);
    }
}

std::pair<std::string, std::string> Lidar::getscaninfo(Scan::scan_t type) const
{
    const auto scan = getscan(type);
    return {scan->gettypename(), scan->getsubtypename()};
}

Lidar::Lidar(seriesid series, speed_t baud, scansinitfunc&& initscans) :
    series{series}, baud{baud}, initscans{std::move(initscans)}
{}

seriesid Lidar::getseries(std::shared_ptr<serial> serialIf)
{
    seriesid series{seriesid::unknown};
    serialIf->write({SCANSTARTFLAG, SCANGETINFOCMD});
    std::vector<uint8_t> resp;
    if (serialIf->read(resp, 27))
    {
        uint8_t modelid = resp[7];
        modelId = modelid;
        uint8_t majormodelid = (modelid >> 4) & 0x0F;

        if (majormodelid >= (uint8_t)seriesid::mmodel)
        {
            series = seriesid::mmodel;
        }
        else if (majormodelid >= (uint8_t)seriesid::tmodel)
        {
            series = seriesid::tmodel;
        }
        else if (majormodelid >= (uint8_t)seriesid::smodel)
        {
            series = seriesid::smodel;
        }
        else if (majormodelid >= (uint8_t)seriesid::cmodel)
        {
            series = seriesid::cmodel;
        }
        else
        {
            series = seriesid::amodel;
        }
    }

    return series;
}

void Lidar::startmotor()
{
    static constexpr uint8_t accessoryBoardCommand = 0xFF;
    static constexpr uint8_t setMotorPwmCommand = 0xF0;
    static constexpr uint8_t getLidarConfigCommand = 0x84;
    static constexpr uint16_t desiredRotationFrequency = 0x0001;
    static constexpr uint16_t fallbackPwm = 600;

    motorPwm.reset();
    if ((modelId >> 4) >= 2)
    {
        std::vector<uint8_t> request{
            SCANSTARTFLAG, accessoryBoardCommand, 4, 0, 0, 0, 0};
        request.push_back(Helpers::getchecksum(request));
        if (serialIf->write(request) != request.size())
        {
            throw std::runtime_error(
                "Cannot query A2 accessory-board motor support");
        }

        std::vector<uint8_t> response;
        const auto responseSize = serialIf->read(response, 11, 500);
        if (responseSize == 11)
        {
            if (response[0] != 0xA5 || response[1] != 0x5A ||
                response[2] != 4 || response[6] != accessoryBoardCommand)
            {
                throw std::runtime_error(
                    "Invalid A2 accessory-board motor-support response");
            }

            const uint32_t supportFlags =
                static_cast<uint32_t>(response[7]) |
                (static_cast<uint32_t>(response[8]) << 8) |
                (static_cast<uint32_t>(response[9]) << 16) |
                (static_cast<uint32_t>(response[10]) << 24);
            if (supportFlags & 0x01)
            {
                std::vector<uint8_t> configRequest{
                    SCANSTARTFLAG,
                    getLidarConfigCommand,
                    4,
                    static_cast<uint8_t>(desiredRotationFrequency),
                    0,
                    0,
                    0};
                configRequest.push_back(Helpers::getchecksum(configRequest));
                if (serialIf->write(configRequest) != configRequest.size())
                {
                    throw std::runtime_error(
                        "Cannot query the A2 motor PWM setting");
                }

                std::vector<uint8_t> configResponse;
                const auto configSize = serialIf->read(configResponse, 15, 500);
                if (configSize != 0 && configSize != 15)
                {
                    throw std::runtime_error(
                        "Incomplete A2 motor PWM configuration response");
                }
                uint16_t pwm = fallbackPwm;
                if (configSize == 15 && configResponse[0] == 0xA5 &&
                    configResponse[1] == 0x5A && configResponse[6] == 0x20 &&
                    configResponse[2] == 8 &&
                    configResponse[7] == desiredRotationFrequency &&
                    configResponse[8] == 0 && configResponse[9] == 0 &&
                    configResponse[10] == 0)
                {
                    const auto configuredPwm = static_cast<uint16_t>(
                        static_cast<uint16_t>(configResponse[13]) |
                        (static_cast<uint16_t>(configResponse[14]) << 8));
                    if (configuredPwm != 0)
                    {
                        pwm = configuredPwm;
                    }
                }

                std::vector<uint8_t> pwmCommand{
                    SCANSTARTFLAG, setMotorPwmCommand, 2,
                    static_cast<uint8_t>(pwm & 0xFF),
                    static_cast<uint8_t>(pwm >> 8)};
                pwmCommand.push_back(Helpers::getchecksum(pwmCommand));
                if (serialIf->write(pwmCommand) != pwmCommand.size())
                {
                    throw std::runtime_error("Cannot start the A2 motor");
                }
                motorPwm = pwm;
                std::this_thread::sleep_for(std::chrono::milliseconds{10});
                return;
            }
        }
        else if (responseSize != 0)
        {
            throw std::runtime_error(
                "Incomplete A2 accessory-board motor-support response");
        }
    }
}

void Lidar::stopmotor()
{
    if (motorPwm)
    {
        std::vector<uint8_t> pwmCommand{SCANSTARTFLAG, 0xF0, 2, 0, 0};
        pwmCommand.push_back(Helpers::getchecksum(pwmCommand));
        if (serialIf->write(pwmCommand) != pwmCommand.size())
        {
            throw std::runtime_error("Cannot stop the A2 motor");
        }
        motorPwm.reset();
        return;
    }
}

void Lidar::getpacket(std::vector<uint8_t>&& req, std::vector<uint8_t>& resp,
                      uint8_t size, bool chsum = false)
{
    if (chsum)
    {
        req.push_back(Helpers::getchecksum(req));
    }
    serialIf->write(req);
    serialIf->read(resp, size);
}

std::shared_ptr<Scan::Interface::Scan> Lidar::getscan(Scan::scan_t type) const
{
    if (auto scan = std::ranges::find_if(
            scans, [type](auto scan) { return scan->gettype() == type; });
        scan != scans.end())
    {
        return *scan;
    }
    throw std::runtime_error("No scan for given type");
}

} // namespace Lidar
