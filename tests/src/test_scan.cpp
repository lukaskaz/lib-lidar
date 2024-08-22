#include "lidarfactory.hpp"
#include "mocks/mock_serial.hpp"

#include "gtest/gtest.h"

using namespace testing;

class TestScan : public testing::Test
{
  protected:
    std::shared_ptr<NiceMock<SerialMock>> serialMock =
        std::make_shared<NiceMock<SerialMock>>();

  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}

    void simulateProperNormalData()
    {
        Sequence seq;
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::amodel << 4;
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .InSequence(seq)
            .WillOnce(
                DoAll(SetArgReferee<0>(rawfwinfo), Return(rawfwinfo.size())));

        auto startscanreadbytes{7};
        EXPECT_CALL(*serialMock, read(_, startscanreadbytes, _))
            .InSequence(seq)
            .WillOnce(Return(startscanreadbytes));

        // simulate angle 225 (watched/for data collection)
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatch{0x3e, 0x99, 0x70, 0xdf, 0x10};
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.end()};
        EXPECT_CALL(*serialMock, read(_, 3, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(3)));

        // simulate angle 227 (not watched/for triggering notification)
        std::vector<uint8_t> rawdataaftermatch{0x3e, 0x65, 0x71, 0x8c, 0x10};
        rawdata = {rawdataaftermatch.begin(), rawdataaftermatch.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataaftermatch.begin(), rawdataaftermatch.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataaftermatch.begin(), rawdataaftermatch.end()};
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 3, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(3)));

        // simulate angle 227 (not watched/for feeding idle data)
        std::vector<uint8_t> rawdataaftermatch1{0x3e, 0x65, 0x71, 0x8c, 0x10};
        rawdata = {rawdataaftermatch1.begin(), rawdataaftermatch1.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataaftermatch1.begin(), rawdataaftermatch1.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataaftermatch1.begin(), rawdataaftermatch1.end()};
        EXPECT_CALL(*serialMock, read(_, 3, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(3)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }

    void simulateWrongSizeNormalData()
    {
        Sequence seq;
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::amodel << 4;
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .InSequence(seq)
            .WillOnce(
                DoAll(SetArgReferee<0>(rawfwinfo), Return(rawfwinfo.size())));

        auto startscanreadbytes{7};
        EXPECT_CALL(*serialMock, read(_, startscanreadbytes, _))
            .InSequence(seq)
            .WillOnce(Return(startscanreadbytes));

        // simulate angle 225 (watched but data improper)
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatch{0x3e, 0x99, 0x70, 0xdf, 0x10};
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatch.begin(), rawdataonmatch.end()};
        EXPECT_CALL(*serialMock, read(_, 3, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
    }
};

TEST_F(TestScan, RunningNormalScanWithProperData_NotifierCalledOnWatchedAngle)
{
    int32_t angletocheck{225};
    SampleData received;
    simulateProperNormalData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(3);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(angletocheck,
                      [&received](auto data) { received = data; });

    ASSERT_NE(received.first, angletocheck);
    lidar->runscan(scan_t::normal);
    usleep(10 * 1000);
    lidar->stopscan();
    EXPECT_EQ(received.first, angletocheck);
}

TEST_F(TestScan, RunningNormalScanWithPacketSizeMismatch_DataInterpreterThrows)
{
    int32_t angletocheck{225};
    SampleData received;
    simulateWrongSizeNormalData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(2);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(angletocheck,
                      [&received](auto data) { received = data; });

    ASSERT_NE(received.first, angletocheck);
    lidar->runscan(scan_t::normal);
    usleep(10 * 1000);
    ASSERT_THROW(lidar->stopscan();, std::runtime_error);
    EXPECT_NE(received.first, angletocheck);
}
