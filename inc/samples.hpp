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

class SamplesGroup
{
  public:
    SamplesGroup(int32_t);
    SamplesGroup(int32_t, int32_t);

    void addnotifier(NotifyFunc&& func);
    bool addsampletonotify(SampleData);
    void notifyandcleanup();
    std::vector<int32_t> getangles() const;

  private:
    static const int32_t supportangles;
    std::map<int32_t, int32_t> angleswithprio;
    std::map<int32_t, Sample> samplestonotify;
    std::vector<NotifyFunc> notifiers;
};

class Observer
{
  public:
    void event(int32_t, NotifyFunc);
    void update(const SampleData& data);

  private:
    std::unordered_map<int32_t, std::shared_ptr<SamplesGroup>> registeredangles;
    UniqueQueue<std::shared_ptr<SamplesGroup>> notifyqueue;
};
