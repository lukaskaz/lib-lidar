#include "samples.hpp"

#include "gtest/gtest.h"

class TestSamplesGroup : public testing::Test
{
  protected:
    void SetUp() override
    {
        samplesgroup = std::make_shared<SamplesGroup>(angle);
        samplesgroup->subscribe(Observer<SampleData>::create(defaultfunc));
    }

    void TearDown() override
    {}

  protected:
    std::shared_ptr<SamplesGroup> samplesgroup;
    const int32_t angle{30};
    const double validdistance{55.f};
    const double invaliddistance{.5f};
    const SampleData validdata{angle, validdistance};
    const SampleData invaliddata{angle, invaliddistance};
    int32_t notifycounter{};
    SampleData notifieddata{};

    Observer<SampleData>::Func defaultfunc{[this](auto& data) {
        notifycounter++;
        notifieddata = data;
    }};
};

TEST_F(TestSamplesGroup, DefaultAnglesGroupCreated_AllAnglesPresent)
{
    std::vector<int32_t> expectedangles = {
        angle - 1,
        angle,
        angle + 1,
    };
    EXPECT_EQ(expectedangles, samplesgroup->getangles());
}

TEST_F(TestSamplesGroup, UpdatedThreeValidSamplesOfGroup_NotifierCalledOnce)
{
    SampleData datasuppprev(validdata.first - 1, validdata.second + 1);
    SampleData datasuppnext(validdata.first + 1, validdata.second + 2);
    samplesgroup->addsampletonotify(datasuppnext);
    samplesgroup->addsampletonotify(validdata);
    samplesgroup->addsampletonotify(datasuppprev);
    ASSERT_NE(notifycounter, 1);
    samplesgroup->notifyandcleanup();
    EXPECT_EQ(notifycounter, 1);
}

TEST_F(TestSamplesGroup, UpdatedThreeInvalidSamplesOfGroup_NotifierNotCalled)
{
    SampleData datasuppprev(invaliddata.first - 1, invaliddata.second);
    SampleData datasuppnext(invaliddata.first + 1, invaliddata.second);
    samplesgroup->addsampletonotify(datasuppnext);
    samplesgroup->addsampletonotify(invaliddata);
    samplesgroup->addsampletonotify(datasuppprev);
    samplesgroup->notifyandcleanup();
    EXPECT_EQ(notifycounter, 0);
}

TEST_F(TestSamplesGroup,
       UpdatedThreeValidSamplesOfGroup_NotifierCalledForHighestPrio)
{
    SampleData datasuppprev(validdata.first - 1, validdata.second + 1);
    SampleData datasuppnext(validdata.first + 1, validdata.second + 2);
    samplesgroup->addsampletonotify(datasuppnext);
    samplesgroup->addsampletonotify(validdata);
    samplesgroup->addsampletonotify(datasuppprev);
    ASSERT_NE(notifieddata, validdata);
    samplesgroup->notifyandcleanup();
    EXPECT_EQ(notifieddata, validdata);
}

TEST_F(TestSamplesGroup,
       UpdatedTwoValidSupporingSamplesOfGroup_NotifierCalledForHighestPrio)
{
    SampleData datasuppprev(validdata.first - 1, validdata.second + 1);
    SampleData datasuppnext(validdata.first + 1, validdata.second + 2);
    samplesgroup->addsampletonotify(datasuppnext);
    samplesgroup->addsampletonotify(datasuppprev);
    ASSERT_NE(notifieddata, datasuppprev);
    samplesgroup->notifyandcleanup();
    EXPECT_EQ(notifieddata, datasuppprev);
}

TEST_F(TestSamplesGroup,
       UpdatedOneValidSupporingSampleOfGroup_NotifierCalledForHighestPrio)
{
    SampleData datasuppnext(validdata.first + 1, validdata.second + 5);
    samplesgroup->addsampletonotify(datasuppnext);
    ASSERT_NE(notifieddata, datasuppnext);
    samplesgroup->notifyandcleanup();
    EXPECT_EQ(notifieddata, datasuppnext);
}

TEST_F(TestSamplesGroup, AddedSeveralNotifiersForGroup_AllNotifiersCalled)
{
    int32_t notifycountersecond{}, notifycounterthird{};
    samplesgroup->subscribe(Observer<SampleData>::create(
        [&notifycountersecond]([[maybe_unused]] auto& data) {
            notifycountersecond++;
        }));
    samplesgroup->subscribe(Observer<SampleData>::create(
        [&notifycounterthird]([[maybe_unused]] auto& data) {
            notifycounterthird++;
        }));

    samplesgroup->addsampletonotify(validdata);
    auto initial =
        std::make_tuple(notifycounter, notifycountersecond, notifycounterthird);
    samplesgroup->notifyandcleanup();

    auto expected = std::make_tuple(1, 1, 1);
    auto received =
        std::make_tuple(notifycounter, notifycountersecond, notifycounterthird);
    ASSERT_NE(expected, initial);
    EXPECT_EQ(expected, received);
}
