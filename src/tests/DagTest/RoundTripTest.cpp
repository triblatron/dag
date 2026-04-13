//
// Created by Tony Horrobin on 04/04/2026.
//

#include "../../../thirdparty/dagbase/include/core/Types.h"
#include <gtest/gtest.h>

class CopyOp_testRoundTrip : public ::testing::TestWithParam<std::tuple<std::string, dagbase::CopyOp>>
{

};

TEST_P(CopyOp_testRoundTrip, testRoundTrip)
{
    auto str = std::get<0>(GetParam());
    auto value = std::get<1>(GetParam());

    EXPECT_EQ(str, dagbase::copyOpToString(value));
    EXPECT_EQ(value, dagbase::parseCopyOp(str));
}

INSTANTIATE_TEST_SUITE_P(CopyOp, CopyOp_testRoundTrip, ::testing::Values(
    std::make_tuple("DEEP_COPY_NONE", dagbase::DEEP_COPY_NONE),
    std::make_tuple("DEEP_COPY_NODES_BIT", dagbase::DEEP_COPY_NODES_BIT),
    std::make_tuple("DEEP_COPY_INPUTS_BIT", dagbase::DEEP_COPY_INPUTS_BIT),
    std::make_tuple("DEEP_COPY_OUTPUTS_BIT", dagbase::DEEP_COPY_OUTPUTS_BIT),
    std::make_tuple("GENERATE_UNIQUE_ID_BIT", dagbase::GENERATE_UNIQUE_ID_BIT),
    std::make_tuple("DEEP_COPY_PARENT_BIT", dagbase::DEEP_COPY_PARENT_BIT),
    std::make_tuple("DEEP_COPY_NODES_BIT DEEP_COPY_INPUTS_BIT DEEP_COPY_OUTPUTS_BIT", static_cast<dagbase::CopyOp>(dagbase::DEEP_COPY_NODES_BIT|dagbase::DEEP_COPY_INPUTS_BIT|dagbase::DEEP_COPY_OUTPUTS_BIT))
    ));
    // DEEP_COPY_INPUTS_BIT    = 1<<1,
    // DEEP_COPY_OUTPUTS_BIT   = 1<<2,
    // GENERATE_UNIQUE_ID_BIT  = 1<<3,
    // DEEP_COPY_PARENT_BIT    = 1<<4
class PortType_testRoundTrip : public ::testing::TestWithParam<std::tuple<const char*, dagbase::PortType::Type>>
{

};

TEST_P(PortType_testRoundTrip, testRoundTrip)
{
    const char* str = std::get<0>(GetParam());
    dagbase::PortType::Type type = std::get<1>(GetParam());
    auto actualType = dagbase::PortType::parseFromString(str);
    EXPECT_STREQ(str, dagbase::PortType::toString(type));
    EXPECT_EQ(type, actualType);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testRoundTrip, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", dagbase::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_UINT8", dagbase::PortType::TYPE_UINT8),
    std::make_tuple("TYPE_INT8", dagbase::PortType::TYPE_INT8),
    std::make_tuple("TYPE_UINT16", dagbase::PortType::TYPE_UINT16),
    std::make_tuple("TYPE_INT16", dagbase::PortType::TYPE_INT16),
    std::make_tuple("TYPE_UINT32", dagbase::PortType::TYPE_UINT32),
    std::make_tuple("TYPE_INT32", dagbase::PortType::TYPE_INT32),
    std::make_tuple("TYPE_UINT64", dagbase::PortType::TYPE_UINT64),
    std::make_tuple("TYPE_INT64", dagbase::PortType::TYPE_INT64),
    std::make_tuple("TYPE_DOUBLE", dagbase::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", dagbase::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", dagbase::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", dagbase::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", dagbase::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", dagbase::PortType::TYPE_VECTOR)
));

