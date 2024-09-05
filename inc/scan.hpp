#pragma once

#include "interfaces/scan.hpp"
#include "samples.hpp"
#include "serial.hpp"

#include <cstdint>
#include <exception>
#include <future>
#include <memory>
#include <vector>

using Measurement = std::pair<bool, SampleData>;

class Scan : public ScanIf, public std::enable_shared_from_this<Scan>
{
  public:
    Scan(std::shared_ptr<serial>, scan_t, scansub_t);
    ~Scan();

    void run() override;
    void stop() override;
    void addangle(int32_t, std::shared_ptr<Observer<SampleData>>) override;
    void delangle(int32_t) override;

    scan_t gettype() const override;
    scansub_t getsubtype() const override;
    std::string gettypename() const override;
    std::string getsubtypename() const override;

    bool isrunning() const override;

  protected:
    SamplesManager samplesmanager;
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

class ScanNormal : public Scan
{
  private:
    friend class ScanFactory;
    ScanNormal(std::shared_ptr<serial> serialIf) :
        Scan(serialIf, scan_t::normal, scansub_t::none)
    {}
    void requestscan() override;
    Measurement getdata(bool);
    void processscan() override;
};

class ScanExpress : public Scan
{
  public:
    ScanExpress(std::shared_ptr<serial> serialIf, scansub_t subtype) :
        Scan(serialIf, scan_t::express, subtype)
    {}

  protected:
    void requestscan() override;
    virtual std::pair<double, std::vector<uint8_t>> getbasedata(bool);
};

class ScanExpressLegacy : public ScanExpress
{
  private:
    friend class ScanFactory;
    ScanExpressLegacy(std::shared_ptr<serial> serialIf) :
        ScanExpress(serialIf, scansub_t::legacy)
    {}
    std::array<Measurement, 2> getcabindata(std::vector<uint8_t>&&, double,
                                            double, uint8_t);
    void processscan() override;
};

class ScanExpressDense : public ScanExpress
{
  private:
    friend class ScanFactory;
    ScanExpressDense(std::shared_ptr<serial> serialIf) :
        ScanExpress(serialIf, scansub_t::dense)
    {}
    Measurement getcabindata(std::vector<uint8_t>&&, double, double, uint8_t);
    void processscan() override;
};
