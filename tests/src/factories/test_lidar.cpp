#include "lidarfactory.hpp"
#include "mocks/mock_serial.hpp"

#include "gtest/gtest.h"

using namespace testing;
using namespace std::string_literals;

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

    void simulateSeries(seriesid series)
    {
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)series << 4;
        ON_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .WillByDefault(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));
    }
};

TEST_F(TestLidarFactory, CreatedAseries_GettingAseriesIdSucceeded)
{
    auto lidar = LidarFactory::createAseries();
    EXPECT_EQ(lidar->getseries(), seriesid::amodel);
    EXPECT_EQ(lidar->getspeed(), (speed_t)B115200);
}

TEST_F(TestLidarFactory, CreatedCseries_GettingAseriesScansSucceeded)
{
    simulateSeries(seriesid::amodel);
    auto lidar = LidarFactory::createAseries();

    ASSERT_TRUE(lidar->setup(serialMock));
    std::pair<std::string, std::string> firstscaninfo, secondscaninfo;
    ASSERT_NO_THROW(({
        firstscaninfo = lidar->getscaninfo(scan_t::normal),
        secondscaninfo = lidar->getscaninfo(scan_t::express);
    }));

    EXPECT_EQ(firstscaninfo, std::make_pair("normal"s, ""s));
    EXPECT_EQ(secondscaninfo, std::make_pair("express"s, "legacy"s));
}

TEST_F(TestLidarFactory, CreatedCseries_GettingCseriesIdSucceeded)
{
    auto lidar = LidarFactory::createCseries();
    EXPECT_EQ(lidar->getseries(), seriesid::cmodel);
    EXPECT_EQ(lidar->getspeed(), (speed_t)B460800);
}

TEST_F(TestLidarFactory, CreatedCseries_GettingCseriesScansSucceeded)
{
    simulateSeries(seriesid::cmodel);
    auto lidar = LidarFactory::createCseries();

    ASSERT_TRUE(lidar->setup(serialMock));
    std::pair<std::string, std::string> firstscaninfo, secondscaninfo;
    ASSERT_NO_THROW(({
        firstscaninfo = lidar->getscaninfo(scan_t::normal),
        secondscaninfo = lidar->getscaninfo(scan_t::express);
    }));

    EXPECT_EQ(firstscaninfo, std::make_pair("normal"s, ""s));
    EXPECT_EQ(secondscaninfo, std::make_pair("express"s, "dense"s));
}

class TestLidarFinder : public TestLidarFactory
{
  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}
};

TEST_F(TestLidarFinder, FindingAvailableLidar_GettingAseriesLidarSucceeded)
{
    simulateSeries(seriesid::amodel);
    ASSERT_NO_THROW(({
        auto lidar = LidarFinder::run(serialMock);
        EXPECT_EQ(lidar->getseries(), seriesid::amodel);
    }));
}

TEST_F(TestLidarFinder, FindingAvailableLidar_GettingCseriesLidarSucceeded)
{
    simulateSeries(seriesid::cmodel);
    ASSERT_NO_THROW(({
        auto lidar = LidarFinder::run(serialMock);
        EXPECT_EQ(lidar->getseries(), seriesid::cmodel);
    }));
}

TEST_F(TestLidarFinder, FindingAvailableLidar_NotGettingLidarThrows)
{
    simulateSeries(seriesid::unknown);
    EXPECT_THROW(auto lidar = LidarFinder::run(serialMock), std::runtime_error);
}
