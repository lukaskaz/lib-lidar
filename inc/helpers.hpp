#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

std::string gettimestr();
uint8_t getchecksum(const std::vector<uint8_t>&);

template <typename T>
class UniqueQueue
{
  public:
    void put(T value)
    {
        if (!set.contains(value))
        {
            set.insert(value);
            queue.push(value);
        }
    }

    T get()
    {
        if (!queue.empty())
        {
            T value = queue.front();
            queue.pop();
            set.erase(value);
            return value;
        }
        throw std::runtime_error("Trying to get element from empty queue");
    }

    bool empty() const
    {
        return queue.empty();
    }

  private:
    std::unordered_set<T> set;
    std::queue<T> queue;
};

template <typename T>
class Observer
{
  public:
    using Func = std::function<void(const T&)>;
    static std::shared_ptr<Observer<T>> create(const Func& func)
    {
        return std::shared_ptr<Observer<T>>(new Observer<T>(func));
    }

    void operator()(const T& param)
    {
        func(param);
    }

  private:
    Observer(const Func& func) : func{func}
    {}
    Func func;
};

template <typename T>
class Observable
{
  public:
    void notify(const T& param)
    {
        std::ranges::for_each(observers, [&param](auto obs) { (*obs)(param); });
    }

    void subscribe(std::shared_ptr<Observer<T>> obs)
    {
        if (!observers.insert(obs).second)
        {
            throw std::runtime_error(
                "Trying to subscribe already existing observer");
        }
    }

    void unsubscribe(std::shared_ptr<Observer<T>> obs)
    {
        if (!observers.erase(obs))
        {
            throw std::runtime_error(
                "Trying to unsubscribe not existing observer");
        }
    }

  private:
    std::unordered_set<std::shared_ptr<Observer<T>>> observers;
};
