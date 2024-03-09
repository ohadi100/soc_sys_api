/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include "sok/common/CsmAccessorDemo.hpp"

using namespace sok::common;

class CsmAccessorDemoTest : public ::testing::Test
{

};

TEST_F(CsmAccessorDemoTest, IsKeyExists)
{
    CsmAccessorDemo csmAccessorDemo;
    EXPECT_EQ(csmAccessorDemo.IsKeyExists(111), CsmErrorCode::kSuccess);
}

TEST_F(CsmAccessorDemoTest, MacVerify)
{
    CsmAccessorDemo csmAccessorDemo;
    std::vector<uint8_t> vector1 = { 1, 2, 3, 4 };
    std::vector<uint8_t> vector2 = { 1, 2, 3, 5 };

    CsmResult<std::vector<uint8_t>> res1 = csmAccessorDemo.MacCreate(111,vector1,MacAlgorithm::kSipHash24);
    CsmResult<std::vector<uint8_t>> res2 = csmAccessorDemo.MacCreate(111,vector2,MacAlgorithm::kSipHash24);

    ASSERT_TRUE(res2.getObject()!=res1.getObject());
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector1,res1.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kSuccess);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector2,res2.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kSuccess);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector1,res2.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kErrorRng);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector2,res1.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kErrorRng);
}

TEST_F(CsmAccessorDemoTest, GenerateRandomBytes)
{
    CsmAccessorDemo csmAccessorDemo;

    CsmResult<std::vector<uint8_t>> vector1 = csmAccessorDemo.GenerateRandomBytes(10);
    CsmResult<std::vector<uint8_t>> vector2 = csmAccessorDemo.GenerateRandomBytes(10);
    CsmResult<std::vector<uint8_t>> mac1 = csmAccessorDemo.MacCreate(111,vector1.getObject(),MacAlgorithm::kSipHash24);
    CsmResult<std::vector<uint8_t>> mac2 = csmAccessorDemo.MacCreate(111,vector2.getObject(),MacAlgorithm::kSipHash24);
    ASSERT_TRUE(mac2.getObject()!=mac1.getObject());
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector1.getObject(),mac1.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kSuccess);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector2.getObject(),mac2.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kSuccess);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector1.getObject(),mac2.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kErrorRng);
    EXPECT_EQ(csmAccessorDemo.MacVerify(111,vector2.getObject(),mac1.getObject(),MacAlgorithm::kSipHash24), CsmErrorCode::kErrorRng);
}