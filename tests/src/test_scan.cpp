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
        std::vector<uint8_t> rawdataforfeeding{0x3e, 0x65, 0x71, 0x8c, 0x10};
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.end()};
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
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 3, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }

    void simulateProperExpressLegacyData()
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

        // simulate angle 145 and succeeding within cabin packages
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatchprev{
            0xad, 0x57, 0x4,  0xA1, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
            0xa3, 0x33, 0xd0, 0xd3, 0x33, 0xf7, 0x33, 0xdd, 0x0,  0x0,  0x9f,
            0x59, 0xe0, 0xd3, 0x5e, 0x33, 0x5a, 0xef, 0xff, 0x58, 0xaf, 0x57,
            0xee, 0xbf, 0x55, 0x87, 0x54, 0xee, 0xc3, 0x53, 0xb,  0x53, 0xee,
            0xcb, 0x51, 0x83, 0x53, 0xee, 0xcf, 0x53, 0x77, 0x40, 0xee, 0x5f,
            0x3f, 0x7b, 0x40, 0xee, 0xdf, 0x40, 0xf3, 0x40, 0xee, 0x83, 0x41,
            0x1b, 0x43, 0xee, 0x7,  0x45, 0x97, 0x45, 0xee, 0xa3, 0x46, 0x97,
            0x47, 0xee, 0x4b, 0x47, 0xef, 0x46, 0xee};
        std::vector<uint8_t> rawdataonmatchnext{
            0xae, 0x5a, 0x8a, 0x26, 0x9b, 0x46, 0x2f, 0x46, 0xee, 0x8f, 0x45,
            0x9b, 0x35, 0xde, 0x67, 0x35, 0xd7, 0x34, 0xdd, 0x97, 0x34, 0x5b,
            0x34, 0xdd, 0x4b, 0x34, 0x7,  0x34, 0xdd, 0xeb, 0x33, 0xd3, 0x33,
            0xdd, 0xa7, 0x34, 0xcf, 0x35, 0xdd, 0x37, 0x33, 0x9b, 0x37, 0xdd,
            0x67, 0x3f, 0xf7, 0x3f, 0xee, 0xb3, 0x3f, 0x73, 0x3f, 0xee, 0x47,
            0x3f, 0xff, 0x3e, 0xee, 0x13, 0x39, 0xd3, 0x35, 0xdd, 0x53, 0x10,
            0x53, 0x10, 0x65, 0x4f, 0x10, 0x0,  0x0,  0x6,  0x0,  0x0,  0x0,
            0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0};
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.end()};
        EXPECT_CALL(*serialMock, read(_, 80, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.end()};
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 80, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        // simulate angle after 145 (not watched/for feeding idle data)
        std::vector<uint8_t> rawdataforfeeding{
            0xa8, 0x53, 0xad, 0x2c, 0xc3, 0x3c, 0xbf, 0x3c, 0xed, 0x9f, 0x3c,
            0x97, 0x3c, 0xed, 0x83, 0x3c, 0xb3, 0x3c, 0xde, 0x77, 0x3c, 0x63,
            0x3c, 0xee, 0x37, 0x3c, 0x5b, 0x3c, 0xde, 0x63, 0x3c, 0x7,  0x3a,
            0xde, 0x7,  0x37, 0x67, 0x36, 0xdd, 0xc7, 0x35, 0xdf, 0x34, 0xdd,
            0x5b, 0x34, 0x9f, 0x33, 0xdd, 0xe3, 0x32, 0x9b, 0x32, 0xdd, 0x97,
            0x31, 0x23, 0x31, 0xdd, 0x8f, 0x30, 0x3,  0x30, 0xdd, 0xb3, 0x2f,
            0x3f, 0x2f, 0xdd, 0x87, 0x2e, 0x27, 0x2e, 0xdd, 0xb3, 0x2d, 0xbb,
            0x2d, 0xcc, 0x57, 0x2e, 0x4f, 0x2f, 0xdd};
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.end()};
        EXPECT_CALL(*serialMock, read(_, 80, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }

    void simulateWrongSizeExpressLegacyData()
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

        // simulate angle 145 and succeeding within cabin packages
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatchprev{
            0xad, 0x57, 0x4,  0xA1, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
            0xa3, 0x33, 0xd0, 0xd3, 0x33, 0xf7, 0x33, 0xdd, 0x0,  0x0,  0x9f,
            0x59, 0xe0, 0xd3, 0x5e, 0x33, 0x5a, 0xef, 0xff, 0x58, 0xaf, 0x57,
            0xee, 0xbf, 0x55, 0x87, 0x54, 0xee, 0xc3, 0x53, 0xb,  0x53, 0xee,
            0xcb, 0x51, 0x83, 0x53, 0xee, 0xcf, 0x53, 0x77, 0x40, 0xee, 0x5f,
            0x3f, 0x7b, 0x40, 0xee, 0xdf, 0x40, 0xf3, 0x40, 0xee, 0x83, 0x41,
            0x1b, 0x43, 0xee, 0x7,  0x45, 0x97, 0x45, 0xee, 0xa3, 0x46, 0x97,
            0x47, 0xee, 0x4b, 0x47, 0xef, 0x46, 0xee};
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.end()};
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 80, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(79)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }

    void simulateProperExpressDenseData()
    {
        Sequence seq;
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::cmodel << 4;
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .Times(2)
            .InSequence(seq)
            .WillRepeatedly(
                DoAll(SetArgReferee<0>(rawfwinfo), Return(rawfwinfo.size())));

        auto startscanreadbytes{7};
        EXPECT_CALL(*serialMock, read(_, startscanreadbytes, _))
            .InSequence(seq)
            .WillOnce(Return(startscanreadbytes));

        // simulate angle 37 and succeeding within cabin packages
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatchprev{
            0xab, 0x53, 0x92, 0x87, 0x2,  0x1, 0xfe, 0x0, 0xfd, 0x0, 0xfc, 0x0,
            0xfb, 0x0,  0xf9, 0x0,  0xfa, 0x0, 0xfc, 0x0, 0xfc, 0x0, 0xfd, 0x0,
            0xff, 0x0,  0x1,  0x1,  0x6,  0x1, 0xc,  0x1, 0x0,  0x0, 0xbb, 0x1,
            0xbd, 0x1,  0xc1, 0x1,  0xc4, 0x1, 0xc7, 0x1, 0xcc, 0x1, 0xd1, 0x1,
            0xd5, 0x1,  0xdb, 0x1,  0xe4, 0x1, 0xeb, 0x1, 0xf2, 0x1, 0xfb, 0x1,
            0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0,  0x0, 0x0,  0x0,
            0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0xf2, 0x0, 0x0,  0x0, 0x0,  0x0,
        };
        std::vector<uint8_t> rawdataonmatchnext{
            0xa1, 0x5c, 0xb7, 0xf, 0x0,  0x0, 0x0,  0x0, 0x0,  0x0, 0x0,  0x0,
            0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0xfd, 0x3, 0xed, 0x3, 0xe1, 0x3,
            0xd7, 0x3,  0xd1, 0x3, 0xc9, 0x3, 0xc2, 0x3, 0x0,  0x0, 0x0,  0x0,
            0x0,  0x0,  0x83, 0x0, 0x82, 0x0, 0x82, 0x0, 0x84, 0x0, 0x83, 0x0,
            0x82, 0x0,  0x83, 0x0, 0x83, 0x0, 0x84, 0x0, 0x83, 0x0, 0x83, 0x0,
            0x84, 0x0,  0x83, 0x0, 0x81, 0x0, 0x81, 0x0, 0x81, 0x0, 0x82, 0x0,
            0x82, 0x0,  0x82, 0x0, 0x83, 0x0, 0x83, 0x0, 0x84, 0x0, 0x84, 0x0};
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.end()};
        EXPECT_CALL(*serialMock, read(_, 80, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchnext.begin(), rawdataonmatchnext.end()};
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 80, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        // simulate angle after 37 (not watched/for feeding idle data)
        std::vector<uint8_t> rawdataforfeeding{
            0xa4, 0x54, 0x3b, 0x17, 0x83, 0x0, 0x84, 0x0, 0x83, 0x0, 0x84, 0x0,
            0x86, 0x0,  0x87, 0x0,  0x86, 0x0, 0x85, 0x0, 0x86, 0x0, 0x86, 0x0,
            0x88, 0x0,  0x89, 0x0,  0x8a, 0x0, 0x89, 0x0, 0x8a, 0x0, 0x8c, 0x0,
            0x8c, 0x0,  0x8c, 0x0,  0x8d, 0x0, 0x8d, 0x0, 0x8e, 0x0, 0x90, 0x0,
            0x91, 0x0,  0x92, 0x0,  0x93, 0x0, 0x93, 0x0, 0x95, 0x0, 0x96, 0x0,
            0x95, 0x0,  0x0,  0x0,  0x9c, 0x0, 0x0,  0x0, 0xe,  0x2, 0x12, 0x2,
            0x18, 0x2,  0x1c, 0x2,  0x19, 0x2, 0xe,  0x2, 0x0,  0x0, 0x77, 0x1};
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataforfeeding.begin(), rawdataforfeeding.end()};
        EXPECT_CALL(*serialMock, read(_, 80, _))
            .After(lastseqcall)
            .WillRepeatedly(DoAll(SetArgReferee<0>(rawdata), Return(80)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }

    void simulateWrongSizeExpressDenseData()
    {
        Sequence seq;
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = (uint8_t)seriesid::cmodel << 4;
        EXPECT_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .Times(2)
            .InSequence(seq)
            .WillRepeatedly(
                DoAll(SetArgReferee<0>(rawfwinfo), Return(rawfwinfo.size())));

        auto startscanreadbytes{7};
        EXPECT_CALL(*serialMock, read(_, startscanreadbytes, _))
            .InSequence(seq)
            .WillOnce(Return(startscanreadbytes));

        // simulate angle 37 and succeeding within cabin packages
        std::vector<uint8_t> rawdata;
        std::vector<uint8_t> rawdataonmatchprev{
            0xab, 0x53, 0x92, 0x87, 0x2,  0x1, 0xfe, 0x0, 0xfd, 0x0, 0xfc, 0x0,
            0xfb, 0x0,  0xf9, 0x0,  0xfa, 0x0, 0xfc, 0x0, 0xfc, 0x0, 0xfd, 0x0,
            0xff, 0x0,  0x1,  0x1,  0x6,  0x1, 0xc,  0x1, 0x0,  0x0, 0xbb, 0x1,
            0xbd, 0x1,  0xc1, 0x1,  0xc4, 0x1, 0xc7, 0x1, 0xcc, 0x1, 0xd1, 0x1,
            0xd5, 0x1,  0xdb, 0x1,  0xe4, 0x1, 0xeb, 0x1, 0xf2, 0x1, 0xfb, 0x1,
            0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0x0,  0x0, 0x0,  0x0, 0x0,  0x0,
            0x0,  0x0,  0x0,  0x0,  0x0,  0x0, 0xf2, 0x0, 0x0,  0x0, 0x0,  0x0,
        };
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 1};
        EXPECT_CALL(*serialMock, read(_, 1, 2000, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 2};
        EXPECT_CALL(*serialMock, read(_, 1, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(1)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.begin() + 4};
        EXPECT_CALL(*serialMock, read(_, 2, _))
            .InSequence(seq)
            .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(2)));
        rawdata = {rawdataonmatchprev.begin(), rawdataonmatchprev.end()};
        auto& lastseqcall =
            EXPECT_CALL(*serialMock, read(_, 80, _))
                .InSequence(seq)
                .WillOnce(DoAll(SetArgReferee<0>(rawdata), Return(79)));

        auto releasescanbytes{250};
        EXPECT_CALL(*serialMock, read(_, releasescanbytes, _))
            .After(lastseqcall)
            .WillOnce(Return(0));
    }
};

TEST_F(TestScan, RunningNormalScanWithProperData_NotifierCalledOnWatchedAngle)
{
    int32_t observedangle{225};
    SampleData receiveddata;
    SampleData expecteddata{observedangle, 107.975};
    simulateProperNormalData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(3);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_NE(receiveddata, expecteddata);
    lidar->runscan(scan_t::normal);
    usleep(10 * 1000);
    lidar->stopscan();
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}

TEST_F(TestScan, RunningNormalScanWithPacketSizeMismatch_DataInterpreterThrows)
{
    int32_t observedangle{225};
    SampleData receiveddata;
    SampleData expecteddata{0, 0.f};
    SampleData received;
    simulateWrongSizeNormalData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(3);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_EQ(receiveddata, expecteddata);
    lidar->runscan(scan_t::normal);
    usleep(10 * 1000);
    ASSERT_THROW(lidar->stopscan();, std::runtime_error);
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}

TEST_F(TestScan,
       RunningExpressLegacyScanWithProperData_NotifierCalledOnWatchedAngle)
{
    int32_t observedangle{145};
    SampleData receiveddata;
    SampleData expecteddata{observedangle, 412.6};
    simulateProperExpressLegacyData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(3);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_NE(receiveddata, expecteddata);
    lidar->runscan(scan_t::express);
    usleep(10 * 1000);
    lidar->stopscan();
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}

TEST_F(TestScan,
       RunningExpressLegacyScanWithPacketSizeMismatch_DataInterpreterThrows)
{
    int32_t observedangle{145};
    SampleData receiveddata;
    SampleData expecteddata{0, 0.f};
    simulateWrongSizeExpressLegacyData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(3);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_EQ(receiveddata, expecteddata);
    lidar->runscan(scan_t::express);
    usleep(10 * 1000);
    ASSERT_THROW(lidar->stopscan();, std::runtime_error);
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}

TEST_F(TestScan,
       RunningExpressDenseScanWithProperData_NotifierCalledOnWatchedAngle)
{
    int32_t observedangle{37};
    SampleData receiveddata;
    SampleData expecteddata{observedangle, 25.2};
    simulateProperExpressDenseData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(4);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_NE(receiveddata, expecteddata);
    lidar->runscan(scan_t::express);
    usleep(10 * 1000);
    lidar->stopscan();
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}

TEST_F(TestScan,
       RunningExpressDenseScanWithPacketSizeMismatch_DataInterpreterThrows)
{
    int32_t observedangle{37};
    SampleData receiveddata;
    SampleData expecteddata{0, 0.f};
    simulateWrongSizeExpressDenseData();
    EXPECT_CALL(*serialMock, write(_, _)).Times(4);
    auto lidar = LidarFinder::run(serialMock);
    lidar->watchangle(observedangle,
                      Observer<SampleData>::create(
                          [&receiveddata](auto data) { receiveddata = data; }));

    ASSERT_EQ(receiveddata, expecteddata);
    lidar->runscan(scan_t::express);
    usleep(10 * 1000);
    ASSERT_THROW(lidar->stopscan();, std::runtime_error);
    EXPECT_THAT(receiveddata, Eq(expecteddata));
}
