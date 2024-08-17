#include "lidarfactory.hpp"
#include "mocks/mock_serial.hpp"

#include "gtest/gtest.h"

using namespace testing;

class TestLidarFactory : public testing::Test
{
  protected:
    std::shared_ptr<NiceMock<SerialMock>> serialMock =
        std::make_shared<NiceMock<SerialMock>>();

  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}
};

TEST_F(TestLidarFactory, CreatedAseries_ReturnsAseriesId)
{
    auto lidar = LidarFactory::createAseries();
    EXPECT_EQ(lidar->getseries(), seriesid::amodel);
    EXPECT_EQ(lidar->getspeed(), B115200);
}

TEST_F(TestLidarFactory, CreatedCseries_ReturnsCseriesId)
{
    auto lidar = LidarFactory::createCseries();
    EXPECT_EQ(lidar->getseries(), seriesid::cmodel);
    EXPECT_EQ(lidar->getspeed(), B460800);
}
