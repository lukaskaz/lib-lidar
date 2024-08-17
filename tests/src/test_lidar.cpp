#include "lidarfactory.hpp"
#include "mocks/mock_serial.hpp"

#include "gtest/gtest.h"

using namespace testing;
using namespace std::string_literals;

class TestLidarInfo : public testing::Test
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

    void simulateInfo()
    {
        std::vector<uint8_t> rawfwinfo = {
            0xa5, 0x5a, 0x14, 0x0,  0x0,  0x0,  0x4,  0x41, 0x1,
            0x1,  0x12, 0x89, 0x32, 0xe1, 0xf4, 0xc2, 0xe3, 0x98,
            0xc4, 0xbc, 0xea, 0x9a, 0xf3, 0x7,  0xe9, 0x48, 0x2};
        ON_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .WillByDefault(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));
    }

    void simulateSamplerate()
    {
        std::vector<uint8_t> rawsamplerate = {0xa5, 0x5a, 0x4, 0x0,  0x0, 0x0,
                                              0x15, 0xf4, 0x1, 0xfa, 0x00};
        ON_CALL(*serialMock, read(_, rawsamplerate.size(), _))
            .WillByDefault(
                DoAll(SetArgReferee<0>(rawsamplerate), Return(true)));
    }
};

#include <iostream>

TEST_F(TestLidarInfo, GettingNameFromLidar_ProperNameReturned)
{
    // simulateSeries(seriesid::amodel);
    simulateInfo();
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(1);
    EXPECT_EQ(lidar->getname(), "C1M1");
}

TEST_F(TestLidarInfo, GettingFwInfoFromLidar_ProperFwInfoReturned)
{
    simulateInfo();
    auto expected = std::make_tuple("0x41"s, "1.1"s, "0x12"s,
                                    "8932e1f4c2e398c4bcea9af37e9482"s);
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(1);
    auto received = lidar->getfwinfo();
    EXPECT_EQ(expected, received);
}

TEST_F(TestLidarInfo, GettingSamplerateFromLidar_ProperSampleratesReturned)
{
    simulateSeries(seriesid::amodel);
    simulateSamplerate();
    std::pair<uint16_t, uint16_t> expected(500, 250);
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(1);
    auto received = lidar->getsamplerate();
    EXPECT_EQ(expected, received);
}

class TestLidarState : public TestWithParam<state>
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

    void simulateState(state val)
    {
        std::vector<uint8_t> rawstatus(10, 0);
        rawstatus[7] = (uint8_t)val;
        ON_CALL(*serialMock, read(_, rawstatus.size(), _))
            .WillByDefault(DoAll(SetArgReferee<0>(rawstatus), Return(true)));
    }
};

TEST_P(TestLidarState, GettingStateFromLidar_ProperStateReturned)
{
    auto expectedstate = GetParam();
    simulateSeries(seriesid::amodel);
    simulateState(expectedstate);
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(1);
    auto [receivedstate, _] = lidar->getstate();
    EXPECT_EQ(expectedstate, receivedstate);
}

INSTANTIATE_TEST_SUITE_P(_, TestLidarState,
                         ::testing::Values(state::good, state::warn,
                                           state::err));
