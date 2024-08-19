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

    void simulateInfo()
    {
        std::vector<uint8_t> rawfwinfo = {
            0xa5, 0x5a, 0x14, 0x0,  0x0,  0x0,  0x4,  0x41, 0x1,
            0x1,  0x12, 0x89, 0x32, 0xe1, 0xf4, 0xc2, 0xe3, 0x98,
            0xc4, 0xbc, 0xea, 0x9a, 0xf3, 0x7,  0xe9, 0x48, 0x2};
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .Times(3)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));
    }

    void simulateSamplerate()
    {
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::amodel << 4;
        auto& firstcall =
            EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
                .WillOnce(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));

        std::vector<uint8_t> rawsamplerate = {0xa5, 0x5a, 0x4, 0x0,  0x0, 0x0,
                                              0x15, 0xf4, 0x1, 0xfa, 0x00};
        EXPECT_CALL(*serialMock, read(_, rawsamplerate.size(), _))
            .After(firstcall)
            .WillOnce(DoAll(SetArgReferee<0>(rawsamplerate), Return(true)));
    }

    void simulateConfiguration()
    {
        InSequence seq;
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::amodel << 4;
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .WillOnce(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));

        std::vector<uint8_t> rawmodecounts = {0xa5, 0x5a, 0x6,  0x0, 0x0,
                                              0x0,  0x20, 0x70, 0x0, 0x0,
                                              0x0,  0x2,  0x0};
        EXPECT_CALL(*serialMock, read(_, rawmodecounts.size(), _))
            .WillOnce(DoAll(SetArgReferee<0>(rawmodecounts), Return(true)));

        std::vector<uint8_t> rawtypicalmode = {0xa5, 0x5a, 0x6,  0x0, 0x0,
                                               0x0,  0x20, 0x7c, 0x0, 0x0,
                                               0x0,  0x0,  0x0};
        EXPECT_CALL(*serialMock, read(_, rawtypicalmode.size(), _))
            .WillOnce(DoAll(SetArgReferee<0>(rawtypicalmode), Return(true)));

        uint8_t defaultnamesize = 211;
        std::vector<uint8_t> rawnamefirst = {
            0xa5, 0x5a, 0xd,  0x0,  0x0,  0x0,  0x20, 0x7f, 0x0,  0x0,
            0x0,  0x53, 0x74, 0x61, 0x6e, 0x64, 0x61, 0x72, 0x64, 0x0};
        EXPECT_CALL(*serialMock, read(_, defaultnamesize, _))
            .WillOnce(DoAll(SetArgReferee<0>(rawnamefirst), Return(true)));

        std::vector<uint8_t> costpersamplefirst = {0xa5, 0x5a, 0x8,  0x0, 0x0,
                                                   0x0,  0x20, 0x71, 0x0, 0x0,
                                                   0x0,  0x0,  0xf4, 0x1, 0x0};
        EXPECT_CALL(*serialMock, read(_, costpersamplefirst.size(), _))
            .WillOnce(
                DoAll(SetArgReferee<0>(costpersamplefirst), Return(true)));

        std::vector<uint8_t> maxdistancefirst = {0xa5, 0x5a, 0x8,  0x0, 0x0,
                                                 0x0,  0x20, 0x74, 0x0, 0x0,
                                                 0x0,  0x0,  0xc,  0x0, 0x0};
        EXPECT_CALL(*serialMock, read(_, maxdistancefirst.size(), _))
            .WillOnce(DoAll(SetArgReferee<0>(maxdistancefirst), Return(true)));

        std::vector<uint8_t> answercmdtypefirst = {
            0xa5, 0x5a, 0x5, 0x0, 0x0, 0x0, 0x20, 0x75, 0x0, 0x0, 0x0, 0x81};
        EXPECT_CALL(*serialMock, read(_, answercmdtypefirst.size(), _))
            .WillOnce(
                DoAll(SetArgReferee<0>(answercmdtypefirst), Return(true)));

        std::vector<uint8_t> rawnamesecond = {
            0xa5, 0x5a, 0xc,  0x0,  0x0,  0x0,  0x20, 0x7f, 0x0, 0x0,
            0x0,  0x45, 0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x0};
        EXPECT_CALL(*serialMock, read(_, defaultnamesize, _))
            .WillOnce(DoAll(SetArgReferee<0>(rawnamesecond), Return(true)));

        std::vector<uint8_t> costpersamplesecond = {0xa5, 0x5a, 0x8,  0x0, 0x0,
                                                    0x0,  0x20, 0x71, 0x0, 0x0,
                                                    0x0,  0x0,  0xfa, 0x0, 0x0};
        EXPECT_CALL(*serialMock, read(_, costpersamplesecond.size(), _))
            .WillOnce(
                DoAll(SetArgReferee<0>(costpersamplesecond), Return(true)));

        std::vector<uint8_t> maxdistancesecond = {0xa5, 0x5a, 0x8,  0x0, 0x0,
                                                  0x0,  0x20, 0x74, 0x0, 0x0,
                                                  0x0,  0x0,  0xc,  0x0, 0x0};
        EXPECT_CALL(*serialMock, read(_, maxdistancesecond.size(), _))
            .WillOnce(DoAll(SetArgReferee<0>(maxdistancesecond), Return(true)));

        std::vector<uint8_t> answercmdtypesecond = {
            0xa5, 0x5a, 0x5, 0x0, 0x0, 0x0, 0x20, 0x75, 0x0, 0x0, 0x0, 0x82};
        EXPECT_CALL(*serialMock, read(_, answercmdtypesecond.size(), _))
            .WillOnce(
                DoAll(SetArgReferee<0>(answercmdtypesecond), Return(true)));
    }
};

TEST_F(TestLidarInfo, GettingNameFromLidar_ProperNameReturned)
{
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
    simulateSamplerate();
    std::pair<uint16_t, uint16_t> expected(500, 250);
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(1);
    auto received = lidar->getsamplerate();
    EXPECT_EQ(expected, received);
}

TEST_F(TestLidarInfo, GettingConfigurationFromLidar_ProperConfigurationReturned)
{
    simulateConfiguration();
    Configuration expected{.modecnt = 2,
                           .typical = 0,
                           .modes = {{0, "Standard", 500, 2000, 12, 0x81},
                                     {1, "Express", 250, 4000, 12, 0x82}}};
    auto lidar = LidarFinder::run(serialMock);

    EXPECT_CALL(*serialMock, write(_, _)).Times(10);
    auto received = lidar->getconfiguration();
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
