#include "samples/samples.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <stdexcept>

#define MAXANGLEPERSCAN 360

namespace Lidar
{

namespace Samples
{

Sample::Sample(Data data) :
    angle{data.first}, distance{getverified(data.second)}
{}

Data Sample::get() const
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

Group::Group(int32_t angle) : Group(angle, supportangles)
{}

const int32_t Group::supportangles{2};

Group::Group(int32_t mainangle, int32_t supportangles)
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

bool Group::addsampletonotify(Data data)
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

void Group::notifyandcleanup()
{
    if (!samplestonotify.empty())
    {
        auto primesample = samplestonotify.begin()->second;
        notify(primesample.get());
        samplestonotify.clear();
    }
}

std::vector<int32_t> Group::getangles() const
{
    auto angles = std::views::keys(angleswithprio);
    return {angles.begin(), angles.end()};
}

void Manager::event(int32_t angle, std::shared_ptr<Helpers::Observer<Data>> obs)
{
    if (registeredangles.contains(angle))
    {
        auto group = registeredangles.at(angle);
        group->subscribe(obs);
    }
    else
    {
        auto group = std::make_shared<Group>(angle);
        std::ranges::for_each(group->getangles(), [this, group](auto angle) {
            if (registeredangles.contains(angle))
            {
                throw std::runtime_error(
                    "Trying to add already registered angle");
            }
            registeredangles.emplace(angle, group);
        });
        group->subscribe(obs);
    }
}

void Manager::update(const Data& data)
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

} // namespace Samples

} // namespace Lidar
