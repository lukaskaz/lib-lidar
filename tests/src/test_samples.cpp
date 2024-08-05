#include "samples.hpp"

#include <cmath>
#include <memory>

#include "gtest/gtest.h"

class TestSamples : public testing::Test
{
  public:
    std::shared_ptr<Sample> sample;
    std::shared_ptr<std::vector<NotifyFunc>> notifiers{
        new std::vector<NotifyFunc>{{[this](const SampleData& data) {
            notifiedvalue = std::get<double>(data);
        }}}};

  protected:
    const int32_t angle{30};
    const double validdistance{55.f};
    const double invaliddistance{.5f};
    double notifiedvalue{};

    void SetUp() override
    {
        sample = std::make_shared<Sample>(angle, notifiers);
    }

    void TearDown() override
    {}
};

TEST_F(TestSamples, IsValidAfterValidValueUpdate)
{
    auto value = validdistance;
    sample->update(value);
    EXPECT_TRUE(sample->isvalid());
}

TEST_F(TestSamples, IsInvalidAfterInvalidValueUpdate)
{
    auto value = invaliddistance;
    sample->update(value);
    EXPECT_FALSE(sample->isvalid());
}

TEST_F(TestSamples, IsSingleNotifiedAfterValidValueUpdate)
{
    auto value = validdistance;
    sample->update(value);
    EXPECT_EQ(value, notifiedvalue);
}

TEST_F(TestSamples, IsTwiceNotifiedAfterValidValueUpdate)
{
    double secondnotifiedvalue{};
    notifiers->push_back([&secondnotifiedvalue](auto& data) {
        secondnotifiedvalue = std::get<double>(data);
    });
    auto value = validdistance;
    sample->update(value);
    ASSERT_NE(value, double{});
    EXPECT_EQ(value, notifiedvalue);
    EXPECT_EQ(value, secondnotifiedvalue);
}

TEST_F(TestSamples, IsNotNotifiedAfterInvalidValueUpdate)
{
    auto value = invaliddistance;
    sample->update(value);
    ASSERT_NE(value, notifiedvalue);
    EXPECT_EQ(notifiedvalue, double{});
}

TEST_F(TestSamples, IsInvalidAfterValidValueUpdate)
{
    auto value = invaliddistance;
    sample->update(value);
    ASSERT_FALSE(std::isnan(value));
    EXPECT_TRUE(std::isnan(std::get<double>(sample->get())));
}

TEST_F(TestSamples, IsAfterValidValueUpdateProperlyReset)
{
    auto value = validdistance;
    sample->update(value);
    ASSERT_TRUE(sample->isvalid());
    sample->reset();
    EXPECT_FALSE(sample->isvalid());
}
