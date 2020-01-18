#include "TestHelpers.h"
#include "CpuUsage.h"

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
