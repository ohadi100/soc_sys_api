/* Copyright (c) 2023 Volkswagen Group */

#include <gtest/gtest.h>
#include <thread>
#include "sok/fvm/SignalManagerDemo.hpp"

using namespace sok::fvm;
using namespace sok::common;

std::vector<uint8_t> value1;
std::vector<uint8_t> value2;

void callback1(std::string const& signal, std::vector<uint8_t> const& value) {
    std::cout << "callback1";
    value1 =value;
}

void callback2(std::string const& signal, std::vector<uint8_t> const& value) {
    std::cout << "callback2";
    value2 = value;
}
class SignalManagerDemoTest : public ::testing::Test
{
public:
};

TEST_F(SignalManagerDemoTest, subscribeAndPublish)
{
    value1 = {};
    value2 = {};
    std::vector<uint8_t> valueOfThread1={1,2,3,4};
    std::vector<uint8_t> valueOfThread2={4,3,2,1};

    std::string name1 = "test1";
    std::string name2 = "test2";
    EXPECT_EQ(FvmErrorCode::kSuccess, SignalManagerDemo::GetInstance()->Subscribe(name1,callback1));
    EXPECT_EQ(FvmErrorCode::kSuccess, SignalManagerDemo::GetInstance()->Subscribe(name2,callback2));
    SignalManagerDemo::GetInstance()->Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    SignalManagerDemo::GetInstance()->Publish(name2,valueOfThread2);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(valueOfThread2,value2);
    EXPECT_EQ(value1, std::vector<uint8_t>{});

    SignalManagerDemo::GetInstance()->Publish(name1,valueOfThread1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(valueOfThread1,value1);
    EXPECT_TRUE(value1!=value2);
}
