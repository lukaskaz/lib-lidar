#include "lidar/factory.hpp"
#include "lidar/finder.hpp"
#include "mocks/mock_serial.hpp"

#include "gtest/gtest.h"

using namespace testing;
using namespace std::string_literals;
using namespace Lidar;

constexpr Lidar::id_t a1m8{seriesid::amodel, 1, 8};
constexpr Lidar::id_t a2m12{seriesid::amodel, 2, 12};
constexpr Lidar::id_t c1m1{seriesid::cmodel, 1, 1};

using LidarFactoryParam =
    std::tuple<Lidar::id_t, speed_t,
               std::shared_ptr<::Lidar::Interface::Lidar> (*)()>;

class TestLidarFactory : public testing::Test
{
  public:
    static std::string to_string(const Lidar::id_t& id)
    {
        std::string series;
        switch (id.series)
        {
            case seriesid::amodel:
                series = "A";
                break;
            case seriesid::cmodel:
                series = "C";
                break;
            case seriesid::smodel:
                series = "S";
                break;
            case seriesid::tmodel:
                series = "T";
                break;
            case seriesid::mmodel:
                series = "M";
                break;
            default:
                series = "Unknown";
                break;
        }

        return series + std::to_string(id.model) + "M" +
               std::to_string(id.submodel);
    }

  protected:
    std::shared_ptr<NiceMock<SerialMock>> serialMock =
        std::make_shared<NiceMock<SerialMock>>();

  protected:
    void SetUp() override
    {}

    void TearDown() override
    {}

    void simulateModel(uint8_t model)
    {
        std::vector<uint8_t> rawfwinfo(27, 0);
        rawfwinfo[7] = model;
        ON_CALL(*serialMock, read(_, rawfwinfo.size(), _))
            .WillByDefault(DoAll(SetArgReferee<0>(rawfwinfo), Return(true)));
    }
};

class TestAseriesLidarFactory :
    public TestLidarFactory,
    public testing::WithParamInterface<LidarFactoryParam>
{};

class TestCseriesLidarFactory :
    public TestLidarFactory,
    public testing::WithParamInterface<LidarFactoryParam>
{};

TEST_P(TestAseriesLidarFactory, CreatedWithExpectedIdAndSpeed)
{
    const auto& [expectedId, expectedSpeed, create] = GetParam();
    const auto lidar = create();

    ASSERT_NE(lidar, nullptr);
    EXPECT_EQ(lidar->getid(), expectedId);
    EXPECT_EQ(lidar->getspeed(), expectedSpeed);
}

TEST_P(TestAseriesLidarFactory, CreatedWithExpectedAseriesScans)
{
    const auto& params = GetParam();
    const auto& expectedId = std::get<0>(params);
    const auto modelId =
        (uint8_t)((expectedId.model << 4) | expectedId.submodel);
    const auto lidar = std::get<2>(params)();

    simulateModel(modelId);
    ASSERT_NE(lidar, nullptr);
    ASSERT_TRUE(lidar->setup(serialMock));
    std::pair<std::string, std::string> firstscaninfo, secondscaninfo;
    ASSERT_NO_THROW(({
        firstscaninfo = lidar->getscaninfo(Scan::scan_t::normal),
        secondscaninfo = lidar->getscaninfo(Scan::scan_t::express);
    }));

    EXPECT_EQ(firstscaninfo, std::make_pair("normal"s, ""s));
    EXPECT_EQ(secondscaninfo, std::make_pair("express"s, "legacy"s));
}

INSTANTIATE_TEST_SUITE_P(
    Aseries, TestAseriesLidarFactory,
    Values(std::make_tuple(a1m8, static_cast<speed_t>(B115200),
                           &Factory::create<a1m8>),
           std::make_tuple(a2m12, static_cast<speed_t>(B256000),
                           &Factory::create<a2m12>)),
    [](const testing::TestParamInfo<LidarFactoryParam>& info) {
        return TestLidarFactory::to_string(std::get<0>(info.param));
    });

TEST_P(TestCseriesLidarFactory, CreatedWithExpectedIdAndSpeed)
{
    const auto& [expectedId, expectedSpeed, create] = GetParam();
    const auto lidar = create();

    ASSERT_NE(lidar, nullptr);
    EXPECT_EQ(lidar->getid(), expectedId);
    EXPECT_EQ(lidar->getspeed(), expectedSpeed);
}

TEST_P(TestCseriesLidarFactory, CreatedWithExpectedCseriesScans)
{
    simulateModel(0x41);
    const auto lidar = std::get<2>(GetParam())();

    ASSERT_NE(lidar, nullptr);
    ASSERT_TRUE(lidar->setup(serialMock));
    std::pair<std::string, std::string> firstscaninfo, secondscaninfo;
    ASSERT_NO_THROW(({
        firstscaninfo = lidar->getscaninfo(Scan::scan_t::normal),
        secondscaninfo = lidar->getscaninfo(Scan::scan_t::express);
    }));

    EXPECT_EQ(firstscaninfo, std::make_pair("normal"s, ""s));
    EXPECT_EQ(secondscaninfo, std::make_pair("express"s, "dense"s));
}

INSTANTIATE_TEST_SUITE_P(
    Cseries, TestCseriesLidarFactory,
    Values(std::make_tuple(c1m1, static_cast<speed_t>(B460800),
                           &Factory::create<c1m1>)),
    [](const testing::TestParamInfo<LidarFactoryParam>& info) {
        return TestLidarFactory::to_string(std::get<0>(info.param));
    });

class TestLidarFinder : public TestLidarFactory
{};

class TestLidarFinderParam :
    public TestLidarFactory,
    public testing::WithParamInterface<Lidar::id_t>
{};

TEST_P(TestLidarFinderParam, FindingAvailableLidarWithExpectedId)
{
    const auto expectedId = GetParam();
    const auto offset = expectedId.series == seriesid::amodel
                            ? 0
                            : (uint8_t)expectedId.series - 1;
    const auto majorId = (uint8_t)(expectedId.model + offset);
    const auto modelId = (uint8_t)((majorId << 4) | expectedId.submodel);

    simulateModel(modelId);
    ASSERT_NO_THROW(({
        auto lidar = Finder::run(serialMock);
        ASSERT_NE(lidar, nullptr);
        EXPECT_EQ(lidar->getid(), expectedId);
    }));
}

INSTANTIATE_TEST_SUITE_P(SupportedModels, TestLidarFinderParam,
                         Values(a1m8, a2m12, c1m1),
                         [](const testing::TestParamInfo<Lidar::id_t>& info) {
                             return TestLidarFactory::to_string(info.param);
                         });

TEST_F(TestLidarFinder, FindingAvailableLidar_NotGettingLidarThrows)
{
    simulateModel(0xFF);
    EXPECT_THROW(auto lidar = Finder::run(serialMock), std::runtime_error);
}
