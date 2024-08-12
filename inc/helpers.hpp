#pragma once

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
