#pragma once
#include <gtest/gtest.h>

class CTestHelpers : public ::testing::Test
{
protected:
    virtual void SetUp();
    virtual void TearDown();
};