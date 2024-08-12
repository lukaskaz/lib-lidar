#include "samples.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <stdexcept>

#define MAXANGLEPERSCAN 360

Sample::Sample(SampleData data) :
    angle{data.first}, distance{getverified(data.second)}
{}

SampleData Sample::get() const
{
    return {angle, distance};
}

bool Sample::isvalid() const
{
    return !std::isnan(distance);
}

double Sample::getverified(double distance) const
{
    static const double invaliddistance{1.f};
    static const auto invalid =
        std::numeric_limits<decltype(distance)>::quiet_NaN();
    return distance < invaliddistance ? invalid : distance;
}

SamplesGroup::SamplesGroup(int32_t angle) : SamplesGroup(angle, supportangles)
{}

const int32_t SamplesGroup::supportangles{2};

SamplesGroup::SamplesGroup(int32_t mainangle, int32_t supportangles)
{
    int32_t prio{1};
    angleswithprio.emplace(mainangle, prio++);
    for (int32_t num{1}; num <= supportangles / 2; num++)
    {
        auto prevsuppangle = mainangle - num < 0
                                 ? MAXANGLEPERSCAN + mainangle - num
                                 : mainangle - num;
        auto nextsuppangle = mainangle + num >= MAXANGLEPERSCAN
                                 ? mainangle + num - MAXANGLEPERSCAN
                                 : mainangle + num;
        angleswithprio.emplace(prevsuppangle, prio++);
        angleswithprio.emplace(nextsuppangle, prio++);
    }
}

void SamplesGroup::addnotifier(NotifyFunc&& func)
{
    notifiers.push_back(std::move(func));
}

bool SamplesGroup::addsampletonotify(SampleData data)
{
    Sample sample{data};
    if (sample.isvalid())
    {
        auto angle = sample.get().first;
        auto prio = angleswithprio.at(angle);
        auto [_, isemplaced] = samplestonotify.emplace(prio, sample);
        return isemplaced;
    }
    return false;
}

void SamplesGroup::notifyandcleanup()
{
    if (!samplestonotify.empty())
    {
        const auto& primesample = samplestonotify.begin()->second;
        std::ranges::for_each(notifiers, [&primesample](auto& notifier) {
            notifier(primesample.get());
        });
        samplestonotify.clear();
    }
}

std::vector<int32_t> SamplesGroup::getangles() const
{
    auto angles = std::views::keys(angleswithprio);
    return {angles.begin(), angles.end()};
}

void Observer::event(int32_t angle, NotifyFunc func)
{
    if (registeredangles.contains(angle))
    {
        auto group = registeredangles.at(angle);
        group->addnotifier(std::move(func));
    }
    else
    {
        auto group = std::make_shared<SamplesGroup>(angle);
        std::ranges::for_each(group->getangles(), [this, group](auto angle) {
            if (registeredangles.contains(angle))
            {
                throw std::runtime_error(
                    "Trying to add already registered angle");
            }
            registeredangles.emplace(angle, group);
        });
        group->addnotifier(std::move(func));
    }
}

void Observer::update(const SampleData& data)
{
    auto [angle, _] = data;
    if (registeredangles.contains(angle))
    {
        auto group = registeredangles.at(angle);
        if (group->addsampletonotify(data))
        {
            notifyqueue.put(group);
        }
    }
    else
    {
        while (!notifyqueue.empty())
        {
            auto group = notifyqueue.get();
            group->notifyandcleanup();
        }
    }
}
