#pragma once

#include "interfaces/samples.hpp"

#include <map>
#include <memory>
#include <vector>

using UpdateFunc = std::function<void(double)>;

class Sample
{
  public:
    explicit Sample(int32_t, std::shared_ptr<std::vector<NotifyFunc>>);

    void reset();
    void update(double);
    bool isvalid() const;
    std::pair<int32_t, double> get() const;

  private:
    static const double invaliddistance;
    const int32_t angle;
    double distance;
    std::shared_ptr<std::vector<NotifyFunc>> notifiers;

    double getinvalid() const;
};

class SampleMonitor
{
  public:
    SampleMonitor(int32_t);
    SampleMonitor(int32_t, int32_t);

    void addnotifier(NotifyFunc&& func);
    std::vector<Sample>& get();

  private:
    static const int32_t defaultsupportnum;
    std::shared_ptr<std::vector<NotifyFunc>> notifiers{
        std::make_shared<std::vector<NotifyFunc>>()};
    std::vector<Sample> samples;
};

class Observer
{
  public:
    Observer();

    void event(int32_t, NotifyFunc);
    void update(const SampleData& data);

  private:
    std::unordered_map<int32_t, SampleMonitor> samples;
    std::unordered_map<int32_t, UpdateFunc> updater;
};
