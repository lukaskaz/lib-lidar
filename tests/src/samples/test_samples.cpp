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
    const SampleData validdata{angle, validdistance};
    const SampleData invaliddata{angle, invaliddistance};

    void SetUp() override
    {}

    void TearDown() override
    {}

    SampleData getData(double distance)
    {
        return {this->angle, distance};
    }
};

TEST_F(TestSample, SampleCreatedWithValidDistance_MarkedAsValid)
{
    EXPECT_TRUE(Sample(validdata).isvalid());
}

TEST_F(TestSample, SampleCreatedWithInvalidDistance_MarkedAsInvalid)
{
    EXPECT_FALSE(Sample(invaliddata).isvalid());
}

TEST_F(TestSample, GetDataFromSampleWithValidDistance_GivesValidData)
{
    EXPECT_EQ(Sample(validdata).get(), validdata);
}

TEST_F(TestSample, GetDataFromSampleWithInvalidDistance_GivesNanDistance)
{
    auto data = Sample(invaliddata).get();
    ASSERT_NE(data, invaliddata);
    EXPECT_EQ(data.first, angle);
    EXPECT_TRUE(std::isnan(data.second));
}
