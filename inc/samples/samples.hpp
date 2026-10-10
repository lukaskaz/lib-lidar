#pragma once

#include "helpers/helpers.hpp"
#include "samples/data.hpp"

#include <map>
#include <memory>
#include <vector>

namespace Lidar
{

namespace Samples
{

class Sample
{
  public:
    explicit Sample(Data);

    bool isvalid() const;
    Data get() const;

  private:
    const int32_t angle;
    const double distance;

    double getverified(double) const;
};

class Group : public Helpers::Observable<Data>
{
  public:
    Group(int32_t);
    Group(int32_t, int32_t);

    bool addsampletonotify(Data);
    void notifyandcleanup();
    std::vector<int32_t> getangles() const;

  private:
    static const int32_t supportangles;
    std::map<int32_t, int32_t> angleswithprio;
    std::map<int32_t, Sample> samplestonotify;
};

class Manager
{
  public:
    void event(int32_t, std::shared_ptr<Helpers::Observer<Data>>);
    void update(const Data&);

  private:
    std::unordered_map<int32_t, std::shared_ptr<Group>> registeredangles;
    Helpers::UniqueQueue<std::shared_ptr<Group>> notifyqueue;
};

} // namespace Samples

} // namespace Lidar
