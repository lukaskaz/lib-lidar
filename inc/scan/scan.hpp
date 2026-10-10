#pragma once

#include "samples/samples.hpp"
#include "scan/interface/scan.hpp"
#include "serial.hpp"

#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <vector>

namespace Lidar
{

namespace Scan
{

class Factory;

constexpr auto MAXANGLEPERSCAN = 360;
constexpr auto SCANSTARTFLAG = 0xA5;
constexpr auto SCANSTARTSCAN = 0x20;
constexpr auto SCANSTOPSCAN = 0x25;

using Measurement = std::pair<bool, Samples::Data>;

class Scan : public Interface::Scan, public std::enable_shared_from_this<Scan>
{
  public:
    Scan(std::shared_ptr<serial>, scan_t, scansub_t);
    ~Scan();

    void run() override;
    void stop() override;
    void addangle(int32_t,
                  std::shared_ptr<Helpers::Observer<Samples::Data>>) override;
    void delangle(int32_t) override;

    scan_t gettype() const override;
    scansub_t getsubtype() const override;
    std::string gettypename() const override;
    std::string getsubtypename() const override;

    bool isrunning() const override;

  protected:
    Samples::Manager samplesmanager;
    const scan_t type;
    const scansub_t subtype;
    std::atomic<bool> running{false};
    std::shared_ptr<serial> serialIf;
    std::exception_ptr exceptptr;
    std::shared_ptr<std::future<void>> scanning;

    virtual void processscan() = 0;
    virtual void requestscan() = 0;
    virtual void releasescan();
};

class Normal : public Scan
{
  private:
    friend class Factory;
    Normal(std::shared_ptr<serial> serialIf) :
        Scan(serialIf, scan_t::normal, scansub_t::none)
    {}
    void requestscan() override;
    Measurement getdata(bool);
    void processscan() override;
};

namespace Base
{

class Express : public Scan
{
  public:
    Express(std::shared_ptr<serial> serialIf, scansub_t subtype) :
        Scan(serialIf, scan_t::express, subtype)
    {}

  protected:
    void requestscan() override;
    virtual std::pair<double, std::vector<uint8_t>> getbasedata(bool = false);
};

} // namespace Base

namespace Legacy
{

class Express : public Base::Express
{
  private:
    friend class Lidar::Scan::Factory;
    Express(std::shared_ptr<serial> serialIf) :
        Base::Express(serialIf, scansub_t::legacy)
    {}
    std::array<Measurement, 2> getcabindata(std::vector<uint8_t>&&, double,
                                            double, uint8_t);
    void processscan() override;
};

} // namespace Legacy

namespace Dense
{

class Express : public Base::Express
{
  private:
    friend class Lidar::Scan::Factory;
    Express(std::shared_ptr<serial> serialIf) :
        Base::Express(serialIf, scansub_t::dense)
    {}
    Measurement getcabindata(std::vector<uint8_t>&&, double, double, uint8_t);
    void processscan() override;
};

} // namespace Dense

} // namespace Scan

} // namespace Lidar
