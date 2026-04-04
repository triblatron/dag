//
// Created by Tony Horrobin on 04/04/2026.
//

#include "Types.h"
#include <gtest/gtest.h>

class CopyOp_testRoundTrip : public ::testing::TestWithParam<std::tuple<std::string, dag::CopyOp>>
{

};

TEST_P(CopyOp_testRoundTrip, testRoundTrip)
{
    auto str = std::get<0>(GetParam());
    auto value = std::get<1>(GetParam());

    EXPECT_EQ(str, dag::copyOpToString(value));
    EXPECT_EQ(value, dag::parseCopyOp(str));
}

INSTANTIATE_TEST_SUITE_P(CopyOp, CopyOp_testRoundTrip, ::testing::Values(
    std::make_tuple("", dag::DEEP_COPY_NONE),
    std::make_tuple("DEEP_COPY_NODES_BIT", dag::DEEP_COPY_NODES_BIT),
    std::make_tuple("DEEP_COPY_INPUTS_BIT", dag::DEEP_COPY_INPUTS_BIT),
    std::make_tuple("DEEP_COPY_OUTPUTS_BIT", dag::DEEP_COPY_OUTPUTS_BIT),
    std::make_tuple("GENERATE_UNIQUE_ID_BIT", dag::GENERATE_UNIQUE_ID_BIT),
    std::make_tuple("DEEP_COPY_PARENT_BIT", dag::DEEP_COPY_PARENT_BIT),
    std::make_tuple("DEEP_COPY_NODES_BIT DEEP_COPY_INPUTS_BIT DEEP_COPY_OUTPUTS_BIT", static_cast<dag::CopyOp>(dag::DEEP_COPY_NODES_BIT|dag::DEEP_COPY_INPUTS_BIT|dag::DEEP_COPY_OUTPUTS_BIT))
    ));
    // DEEP_COPY_INPUTS_BIT    = 1<<1,
    // DEEP_COPY_OUTPUTS_BIT   = 1<<2,
    // GENERATE_UNIQUE_ID_BIT  = 1<<3,
    // DEEP_COPY_PARENT_BIT    = 1<<4
