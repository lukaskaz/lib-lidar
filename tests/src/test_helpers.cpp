#include "helpers.hpp"

#include <algorithm>
#include <limits>
#include <random>

#include "gtest/gtest.h"

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
