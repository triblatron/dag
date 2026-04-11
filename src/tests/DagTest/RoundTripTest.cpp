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
    std::make_tuple("DEEP_COPY_NONE", dag::DEEP_COPY_NONE),
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
class PortType_testRoundTrip : public ::testing::TestWithParam<std::tuple<const char*, dag::PortType::Type>>
{

};

TEST_P(PortType_testRoundTrip, testRoundTrip)
{
    const char* str = std::get<0>(GetParam());
    dag::PortType::Type type = std::get<1>(GetParam());
    auto actualType = dag::PortType::parseFromString(str);
    EXPECT_STREQ(str, dag::PortType::toString(type));
    EXPECT_EQ(type, actualType);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testRoundTrip, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", dag::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_UINT8", dag::PortType::TYPE_UINT8),
    std::make_tuple("TYPE_INT8", dag::PortType::TYPE_INT8),
    std::make_tuple("TYPE_UINT16", dag::PortType::TYPE_UINT16),
    std::make_tuple("TYPE_INT16", dag::PortType::TYPE_INT16),
    std::make_tuple("TYPE_UINT32", dag::PortType::TYPE_UINT32),
    std::make_tuple("TYPE_INT32", dag::PortType::TYPE_INT32),
    std::make_tuple("TYPE_UINT64", dag::PortType::TYPE_UINT64),
    std::make_tuple("TYPE_INT64", dag::PortType::TYPE_INT64),
    std::make_tuple("TYPE_DOUBLE", dag::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", dag::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", dag::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", dag::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", dag::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", dag::PortType::TYPE_VECTOR)
));

