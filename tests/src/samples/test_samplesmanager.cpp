#include "samples.hpp"

#include "gtest/gtest.h"

class TestSamplesManager : public testing::Test
{
  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}

  protected:
    SamplesManager samplesmanager;
    const int32_t angle{30};
    const double validdistance{55.f};
    const double invaliddistance{.5f};
    const SampleData validdata{angle, validdistance};
    const SampleData invaliddata{angle, invaliddistance};
    int32_t notifycounter{};
    SampleData notifieddata;

    Observer<SampleData>::Func defaultfunc{[this](auto& data) {
        notifycounter++;
        notifieddata = data;
    }};
};

TEST_F(TestSamplesManager,
       AddedTwoEventsForNotOverlappingAngles_ObserverNotThrow)
{
    EXPECT_NO_THROW(({
        samplesmanager.event(angle, Observer<SampleData>::create(defaultfunc));
        samplesmanager.event(angle + 3,
                             Observer<SampleData>::create(defaultfunc));
    }));
}

TEST_F(TestSamplesManager, AddedTwoEventsForOverlappingAngles_ObserverThrows)
{
    EXPECT_THROW(({
                     samplesmanager.event(
                         angle, Observer<SampleData>::create(defaultfunc));
                     samplesmanager.event(
                         angle + 2, Observer<SampleData>::create(defaultfunc));
                 }),
                 std::runtime_error);
}

TEST_F(TestSamplesManager, AddedSingleEventForSingleGroupAndUpdated_EventCalled)
{
    auto updatetrigger = SampleData{angle + 2, 0.f};
    samplesmanager.event(angle, Observer<SampleData>::create(defaultfunc));
    samplesmanager.update(validdata);
    ASSERT_NE(notifieddata, validdata);
    samplesmanager.update(updatetrigger);
    EXPECT_EQ(notifieddata, validdata);
}

TEST_F(TestSamplesManager,
       AddedTwoEventsForSingleGroupAndUpdated_BothEventsCalled)
{
    int32_t notifycountersecond{};
    auto updatetrigger = SampleData{angle + 2, 0.f};

    samplesmanager.event(angle, Observer<SampleData>::create(defaultfunc));
    samplesmanager.event(
        angle, Observer<SampleData>::create(
                   [&notifycountersecond]([[maybe_unused]] auto& data) {
                       notifycountersecond++;
                   }));
    samplesmanager.update(validdata);

    auto initial = std::make_tuple(notifycounter, notifycountersecond);
    ASSERT_EQ(notifycounter, 0);
    samplesmanager.update(updatetrigger);

    auto expected = std::make_tuple(1, 1);
    auto received = std::make_tuple(notifycounter, notifycountersecond);
    ASSERT_NE(expected, initial);
    EXPECT_EQ(expected, received);
}

TEST_F(TestSamplesManager,
       AddedSingleEventForSingleGroupAndNotUpdated_EventNotCalled)
{
    auto updatetrigger = SampleData{angle + 2, 0.f};
    samplesmanager.event(angle, Observer<SampleData>::create(defaultfunc));
    ASSERT_EQ(notifycounter, 0);
    samplesmanager.update(updatetrigger);
    EXPECT_EQ(notifycounter, 0);
}

TEST_F(TestSamplesManager,
       AddedSingleEventForSingleGroupAndUpdatedTwice_EventCalledOnce)
{
    SampleData datafirst{50, 20.5f}, datasecond{51, 57.2f};
    auto [mainangle, _] = datafirst;
    auto updatetrigger = SampleData{mainangle + 2, 0.f};

    samplesmanager.event(mainangle, Observer<SampleData>::create(defaultfunc));
    samplesmanager.update(datafirst);
    samplesmanager.update(datasecond);

    ASSERT_EQ(notifycounter, 0);
    samplesmanager.update(updatetrigger);
    EXPECT_EQ(notifycounter, 1);
}

TEST_F(TestSamplesManager,
       AddedTwoEventsForSeparateGroupsAndUpdatedBoth_BothEventsCalled)
{
    SampleData datafirst{50, 20.5f}, datasecond{53, 57.2f};
    auto [firstangle, _1] = datafirst;
    auto [secondangle, _2] = datasecond;
    auto updatetrigger = SampleData{secondangle + 2, 0.f};

    samplesmanager.event(firstangle, Observer<SampleData>::create(defaultfunc));
    samplesmanager.event(secondangle,
                         Observer<SampleData>::create(defaultfunc));
    samplesmanager.update(datafirst);
    samplesmanager.update(datasecond);

    ASSERT_EQ(notifycounter, 0);
    samplesmanager.update(updatetrigger);
    EXPECT_EQ(notifycounter, 2);
}
