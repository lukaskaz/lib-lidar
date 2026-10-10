#pragma once

#include "helpers/helpers.hpp"
#include "samples/data.hpp"
#include "scan/enums.hpp"

#include <cstdint>
#include <string>

namespace Lidar
{

namespace Scan
{

namespace Interface
{

class Scan
{
  public:
    virtual ~Scan()
    {}
    virtual void run() = 0;
    virtual void stop() = 0;
    virtual void
        addangle(int32_t,
                 std::shared_ptr<Helpers::Observer<Samples::Data>>) = 0;
    virtual void delangle(int32_t) = 0;
    virtual scan_t gettype() const = 0;
    virtual scansub_t getsubtype() const = 0;
    virtual std::string gettypename() const = 0;
    virtual std::string getsubtypename() const = 0;
    virtual bool isrunning() const = 0;
};

} // namespace Interface

} // namespace Scan

} // namespace Lidar
