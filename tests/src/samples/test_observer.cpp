#include "samples.hpp"

#include <memory>

#include "gtest/gtest.h"

class TestObserver : public testing::Test
{
  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}

  protected:
    Observer observer;
    const int32_t angle{30};
    const double validdistance{55.f};
    const double invaliddistance{.5f};
    const SampleData validdata{angle, validdistance};
    const SampleData invaliddata{angle, invaliddistance};
    int32_t notifycounter{};
    SampleData notifieddata;

    NotifyFunc defaultnotifier{[this](auto& data) {
        notifycounter++;
        notifieddata = data;
    }};
};

TEST_F(TestObserver, AreTwoDistantAnglesWhenAddedNoThrow)
{
    EXPECT_NO_THROW(({
        observer.event(angle, defaultnotifier);
        observer.event(angle + 3, defaultnotifier);
    }));
}

TEST_F(TestObserver, AreTwoTooCloseAnglesWhenAddedThrow)
{
    EXPECT_THROW(({
                     observer.event(angle, defaultnotifier);
                     observer.event(angle + 2, defaultnotifier);
                 }),
                 std::runtime_error);
}

TEST_F(TestObserver, IsSingleEventProperlyCalledAndHandled)
{
    auto updatetrigger = SampleData{angle + 2, 0.f};
    observer.event(angle, defaultnotifier);
    observer.update(validdata);
    ASSERT_NE(notifieddata, validdata);
    observer.update(updatetrigger);
    EXPECT_EQ(notifieddata, validdata);
}

TEST_F(TestObserver, AreTwoEventsProperlyCalledAndHandled)
{
    int32_t notifycountersecond{};
    auto updatetrigger = SampleData{angle + 2, 0.f};

    observer.event(angle, defaultnotifier);
    observer.event(angle, [&notifycountersecond]([[maybe_unused]] auto& data) {
        notifycountersecond++;
    });
    observer.update(validdata);

    auto initial = std::make_tuple(notifycounter, notifycountersecond);
    ASSERT_EQ(notifycounter, 0);
    observer.update(updatetrigger);

    auto expected = std::make_tuple(1, 1);
    auto received = std::make_tuple(notifycounter, notifycountersecond);
    ASSERT_NE(expected, initial);
    EXPECT_EQ(expected, received);
}

TEST_F(TestObserver, IsSingleEventNotAddedToQueueWhenNoUpdate)
{
    auto updatetrigger = SampleData{angle + 2, 0.f};
    observer.event(angle, defaultnotifier);
    ASSERT_EQ(notifycounter, 0);
    observer.update(updatetrigger);
    EXPECT_EQ(notifycounter, 0);
}

TEST_F(TestObserver, IsSingleEventAddedToQueueOnceOnTwiceUpdate)
{
    SampleData datafirst{50, 20.5f}, datasecond{51, 57.2f};
    auto [mainangle, _] = datafirst;
    auto updatetrigger = SampleData{mainangle + 2, 0.f};

    observer.event(mainangle, defaultnotifier);
    observer.update(datafirst);
    observer.update(datasecond);

    ASSERT_EQ(notifycounter, 0);
    observer.update(updatetrigger);
    EXPECT_EQ(notifycounter, 1);
}
