#include "helpers.hpp"

#include <algorithm>
#include <limits>
#include <random>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace testing;

class TestHelpers : public testing::Test
{
  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}
};

TEST_F(TestHelpers, RequestTimeAndDayDetails_ProvidedStringNotEmpty)
{
    auto timeandday = gettimestr();
    EXPECT_FALSE(timeandday.empty());
}

TEST_F(TestHelpers, CreatedDataPacketNeedingChecksum_ProvidedChecksumCorrect)
{
    std::random_device rd;
    std::uniform_int_distribution<uint8_t> dist(
        std::numeric_limits<uint8_t>::min(),
        std::numeric_limits<uint8_t>::max());
    std::vector<uint8_t> packet(100);

    std::ranges::for_each(packet, [&](auto& byte) { byte = dist(rd); });
    uint8_t receivedchsum{getchecksum(packet)};
    uint8_t expectedchsum{};
    for (const auto& byte : packet)
    {
        expectedchsum = expectedchsum ^ byte;
    }

    EXPECT_EQ(expectedchsum, receivedchsum);
}

TEST_F(TestHelpers, CreatedUniqueQueueAndTryGetFromEmpty_QueueThrows)
{
    UniqueQueue<int32_t> testqueue;
    EXPECT_THROW(testqueue.get(), std::runtime_error);
}

TEST_F(TestHelpers, CreatedUniqueQueueAndPutGetOneElem_ElemPutAndRetrieved)
{
    int32_t userelem = 123;
    UniqueQueue<decltype(userelem)> testqueue;
    testqueue.put(userelem);
    auto recivedelem = testqueue.get();

    EXPECT_EQ(userelem, recivedelem);
}

TEST_F(TestHelpers,
       CreatedUniqueQueueAndPutGetSeveralElems_ElemsPutAndRetrieved)
{
    std::vector<uint32_t> userelems{56, 12, 205};
    UniqueQueue<decltype(userelems)::value_type> testqueue;

    for (const auto& elem : userelems)
    {
        testqueue.put(elem);
    }

    std::vector<uint32_t> recivedelems;
    while (!testqueue.empty())
    {
        recivedelems.push_back(testqueue.get());
    }

    EXPECT_EQ(userelems, recivedelems);
}

TEST_F(TestHelpers,
       CreatedUniqueQueueAndPutTwoSameElems_SingleElemStoredInQueue)
{
    int32_t userelem = 123;
    UniqueQueue<decltype(userelem)> testqueue;
    testqueue.put(userelem);
    testqueue.put(userelem);

    ASSERT_FALSE(testqueue.empty());
    auto recivedelem = testqueue.get();
    ASSERT_TRUE(testqueue.empty());

    EXPECT_EQ(userelem, recivedelem);
}

class TestHelpersObserver : public testing::Test
{
  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}

    struct Data : Observable<Data>
    {
        explicit Data() : Data({}, {})
        {}
        explicit Data(int32_t intval, double floatval) :
            intval{intval}, floatval{floatval}
        {}
        int32_t intval{};
        double floatval{};
        bool operator==(const Data& other) const
        {
            return intval == other.intval && floatval == other.floatval;
        };
    };
};

TEST_F(TestHelpersObserver, SubscribeTwiceSameObserver_ObservableThrows)
{
    uint32_t counter{};
    Data observed;
    auto obs = Observer<Data>::create(
        [&counter]([[maybe_unused]] auto in) { counter++; });

    ASSERT_NO_THROW(observed.subscribe(obs));
    EXPECT_THROW(observed.subscribe(obs), std::runtime_error);
}

TEST_F(TestHelpersObserver, UnsubscribeAbsentObserver_ObservableThrows)
{
    uint32_t counter{};
    Data observed;
    auto obs = Observer<Data>::create(
        [&counter]([[maybe_unused]] auto in) { counter++; });

    EXPECT_THROW(observed.unsubscribe(obs), std::runtime_error);
}

TEST_F(TestHelpersObserver, UnsubscribeTwiceSameObserver_ObservableThrows)
{
    uint32_t counter{};
    Data observed;
    auto obs = Observer<Data>::create(
        [&counter]([[maybe_unused]] auto in) { counter++; });

    ASSERT_NO_THROW(({
        observed.subscribe(obs);
        observed.unsubscribe(obs);
    }));
    EXPECT_THROW(observed.unsubscribe(obs), std::runtime_error);
}

TEST_F(TestHelpersObserver,
       SubscribedObserverNotifiedByData_ObserverGotProperData)
{
    Data observed, given(5, 1.7), received;
    observed.subscribe(Observer<Data>::create(
        [&received]([[maybe_unused]] auto in) { received = in; }));

    ASSERT_THAT(given, Ne(received));
    observed.notify(given);
    EXPECT_THAT(given, Eq(received));
}

TEST_F(TestHelpersObserver,
       UnsubscribedObserverNotifiedOnce_ObserverIsNotCalledWhenUnsubscribed)
{
    uint32_t counter{};
    Data observed;
    auto obs = Observer<Data>::create(
        [&counter]([[maybe_unused]] auto in) { counter++; });

    observed.subscribe(obs);
    ASSERT_THAT(counter, Eq(0));
    observed.notify(observed);
    ASSERT_THAT(counter, Eq(1));

    observed.unsubscribe(obs);
    ASSERT_THAT(counter, Eq(1));
    observed.notify(observed);
    EXPECT_THAT(counter, Eq(1));
}

TEST_F(TestHelpersObserver, SubscribedObserverNotifiedOnce_ObserverIsCalledOnce)
{
    uint32_t counter{};
    Data observed;
    observed.subscribe(Observer<Data>::create(
        [&counter]([[maybe_unused]] auto in) { counter++; }));

    ASSERT_THAT(counter, Eq(0));
    observed.notify(observed);
    EXPECT_THAT(counter, Eq(1));
}

TEST_F(TestHelpersObserver,
       SubscribeObserverAndNotifyTwice_ObserverIsCalledTwice)
{
    uint32_t counter{};
    Data data;
    data.subscribe(Observer<Data>::create(
        [&counter]([[maybe_unused]] auto data) { counter++; }));

    ASSERT_THAT(counter, Eq(0));
    data.notify(data);
    ASSERT_THAT(counter, Eq(1));
    data.notify(data);
    EXPECT_THAT(counter, Eq(2));
}

TEST_F(TestHelpersObserver,
       SubscribeTwoObserversAndNotifyThemOnce_BothObserversAreCalledOnce)
{
    auto counters = std::make_pair<uint32_t, uint32_t>({}, {});
    Data data;
    data.subscribe(Observer<Data>::create(
        [&counters]([[maybe_unused]] auto data) { counters.first++; }));
    data.subscribe(Observer<Data>::create(
        [&counters]([[maybe_unused]] auto data) { counters.second++; }));

    ASSERT_THAT(counters, Eq(std::pair<uint32_t, uint32_t>{0, 0}));
    data.notify(data);
    EXPECT_THAT(counters, Eq(std::pair<uint32_t, uint32_t>{1, 1}));
}
