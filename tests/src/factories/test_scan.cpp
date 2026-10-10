#include "mocks/mock_serial.hpp"
#include "scan/factory.hpp"

#include "gtest/gtest.h"

using namespace testing;
using namespace Lidar::Scan;

class TestScanFactory : public testing::Test
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

TEST_F(TestScanFactory, CreatedNormalScan_ReturnsNormalIdByType)
{
    auto scan = Factory::create<scan_t::normal, scansub_t::none>(serialMock);
    EXPECT_EQ(scan->gettype(), scan_t::normal);
    EXPECT_EQ(scan->getsubtype(), scansub_t::none);
}

TEST_F(TestScanFactory, CreatedNormalScan_ReturnsNormalIdByString)
{
    auto scan = Factory::create<scan_t::normal, scansub_t::none>(serialMock);
    EXPECT_EQ(scan->gettypename(), "normal");
    EXPECT_EQ(scan->getsubtypename(), "");
}

TEST_F(TestScanFactory, CreatedExpressLegacyScan_ReturnsExpressLegacyIdByType)
{
    auto scan = Factory::create<scan_t::express, scansub_t::legacy>(serialMock);
    EXPECT_EQ(scan->gettype(), scan_t::express);
    EXPECT_EQ(scan->getsubtype(), scansub_t::legacy);
}

TEST_F(TestScanFactory, CreatedExpressLegacyScan_ReturnsExpressLegacyIdByString)
{
    auto scan = Factory::create<scan_t::express, scansub_t::legacy>(serialMock);
    EXPECT_EQ(scan->gettypename(), "express");
    EXPECT_EQ(scan->getsubtypename(), "legacy");
}

TEST_F(TestScanFactory, CreatedExpressDenseScan_ReturnsExpressDenseIdByType)
{
    auto scan = Factory::create<scan_t::express, scansub_t::dense>(serialMock);
    EXPECT_EQ(scan->gettype(), scan_t::express);
    EXPECT_EQ(scan->getsubtype(), scansub_t::dense);
}

TEST_F(TestScanFactory, CreatedExpressDenseScan_ReturnsExpressDenseIdByString)
{
    auto scan = Factory::create<scan_t::express, scansub_t::dense>(serialMock);
    EXPECT_EQ(scan->gettypename(), "express");
    EXPECT_EQ(scan->getsubtypename(), "dense");
}
