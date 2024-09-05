#pragma once

#include "helpers.hpp"
#include "interfaces/samples.hpp"

#include <map>
#include <memory>
#include <vector>

class Sample
{
  public:
    explicit Sample(SampleData);

    bool isvalid() const;
    SampleData get() const;

  private:
    const int32_t angle;
    const double distance;

    double getverified(double) const;
};

class SamplesGroup : public Observable<SampleData>
{
  public:
    SamplesGroup(int32_t);
    SamplesGroup(int32_t, int32_t);

    bool addsampletonotify(SampleData);
    void notifyandcleanup();
    std::vector<int32_t> getangles() const;

  private:
    static const int32_t supportangles;
    std::map<int32_t, int32_t> angleswithprio;
    std::map<int32_t, Sample> samplestonotify;
};

class SamplesManager
{
  public:
    void event(int32_t, std::shared_ptr<Observer<SampleData>>);
    void update(const SampleData&);

  private:
    std::unordered_map<int32_t, std::shared_ptr<SamplesGroup>> registeredangles;
    UniqueQueue<std::shared_ptr<SamplesGroup>> notifyqueue;
};
