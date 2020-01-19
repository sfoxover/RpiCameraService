#include "TestHelpers.h"
#include "CpuUsage.h"
#include "helpers.h"

void CTestHelpers::SetUp()
{
}

void CTestHelpers::TearDown()
{
}

// Test cpu usage
TEST(CTestHelpers, TestCpuUsage)
{
    double value = CCpuUsage::GetProcessCpuUsage();

    ASSERT_TRUE(value >= 0) << "CTestHelpers, TestCpuUsage failed test.";
}

// Test cpu usage
TEST(CTestHelpers, TestCpuTemperature)
{
    double value = Helpers::GetCpuTemperature();

    ASSERT_TRUE(value >= 0) << "CTestHelpers, TestCpuTemperature failed test.";
}