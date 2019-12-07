#pragma once
#include <gtest/gtest.h>

class CTestJson : public ::testing::Test 
{
protected:
    virtual void SetUp();
    virtual void TearDown();
};