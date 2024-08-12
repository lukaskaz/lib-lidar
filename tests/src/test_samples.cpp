#include "samples.hpp"

#include <cmath>
#include <memory>

#include "gtest/gtest.h"

class TestSample : public testing::Test
{
  protected:
    const int32_t angle{30};
    const double validdistance{55.f};
    const double invaliddistance{.5f};
    double notifiedvalue{};

    void SetUp() override
    {}

    void TearDown() override
    {}

    SampleData getData(double distance)
    {
        return {this->angle, distance};
    }
};

TEST_F(TestSample, IsValidCreatedWithValidDistance)
{
    Sample sample{angle, validdistance};
    EXPECT_TRUE(sample.isvalid());
}

TEST_F(TestSample, IsValidCreatedWithInvalidDistance)
{
    Sample sample{angle, invaliddistance};
    EXPECT_FALSE(sample.isvalid());
}

TEST_F(TestSample, IsGetDataProperWithValidDistance)
{
    Sample sample{angle, validdistance};
    EXPECT_EQ(sample.get(), SampleData(angle, validdistance));
}

TEST_F(TestSample, IsGetDataProperWithInvalidDistance)
{
    Sample sample{angle, invaliddistance};
    auto data = sample.get();
    ASSERT_NE(data, SampleData(angle, invaliddistance));
    EXPECT_EQ(data.first, angle);
    EXPECT_TRUE(std::isnan(data.second));
}
