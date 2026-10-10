#include "scan/scan.hpp"

#include "helpers/helpers.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <stdexcept>

namespace Lidar
{

namespace Scan
{

Scan::Scan(std::shared_ptr<serial> serialIf, scan_t type, scansub_t subtype) :
    type{type}, subtype{subtype}, serialIf{serialIf}
{}

Scan::~Scan()
{
    stop();
}

void Scan::run()
{
    if (!isrunning())
    {
        running = true;
        scanning = std::make_shared<std::future<void>>(
            std::async(std::launch::async, [weak = weak_from_this()] {
                if (auto self = weak.lock())
                {
                    try
                    {
                        self->requestscan();
                        self->processscan();
                    }
                    catch (...)
                    {
                        self->exceptptr = std::current_exception();
                    }
                    self->releasescan();
                }
            }));
    }
}

void Scan::stop()
{
    if (isrunning())
    {
        running = false;
        scanning->wait();
        if (exceptptr)
        {
            std::rethrow_exception(exceptptr);
        }
    }
}

void Scan::addangle(int32_t angle,
                    std::shared_ptr<Helpers::Observer<Samples::Data>> obs)
{
    samplesmanager.event(angle, obs);
}

void Scan::delangle([[maybe_unused]] int32_t angle)
{}

scan_t Scan::gettype() const
{
    return type;
}

scansub_t Scan::getsubtype() const
{
    return subtype;
}

std::string Scan::gettypename() const
{
    static const std::unordered_map<scan_t, std::string> typetoname = {
        {scan_t::normal, "normal"}, {scan_t::express, "express"}};
    if (typetoname.contains(type))
    {
        return typetoname.at(type);
    }
    throw std::runtime_error("Cannot find scan type to name mappping");
}

std::string Scan::getsubtypename() const
{
    static const std::unordered_map<scansub_t, std::string> subtypetoname = {
        {scansub_t::none, ""},
        {scansub_t::legacy, "legacy"},
        {scansub_t::dense, "dense"}};
    if (subtypetoname.contains(subtype))
    {
        return subtypetoname.at(subtype);
    }
    throw std::runtime_error("Cannot find scan subtype to name mappping");
}

bool Scan::isrunning() const
{
    return running;
}

void Scan::releasescan()
{
    serialIf->write({SCANSTARTFLAG, SCANSTOPSCAN});
    std::vector<uint8_t> resp;
    while (serialIf->read(resp, 250))
        ;
}

} // namespace Scan

} // namespace Lidar
