#include "gtest/gtest.h"

#include "sys-sage.hpp"

TEST(DataPath, Constructors)
{
    Component a, b;
    {
        DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
        EXPECT_EQ(&a, dp.GetSource());
        EXPECT_EQ(&b, dp.GetTarget());
        EXPECT_EQ(SYS_SAGE_DATAPATH_ORIENTED, dp.GetOriented());
        EXPECT_EQ(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, dp.GetDpType());
    }
    {
        DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, 5.0, 42.0};
        EXPECT_EQ(&a, dp.GetSource());
        EXPECT_EQ(&b, dp.GetTarget());
        EXPECT_EQ(SYS_SAGE_DATAPATH_ORIENTED, dp.GetOriented());
        EXPECT_EQ(SYS_SAGE_DATAPATH_TYPE_NONE, dp.GetDpType());
        EXPECT_EQ(5.0, dp.GetBw());
        EXPECT_EQ(42.0, dp.GetLatency());
    }
    {
        DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, 5.0, 42.0};
        EXPECT_EQ(&a, dp.GetSource());
        EXPECT_EQ(&b, dp.GetTarget());
        EXPECT_EQ(SYS_SAGE_DATAPATH_ORIENTED, dp.GetOriented());
        EXPECT_EQ(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, dp.GetDpType());
        EXPECT_EQ(5.0, dp.GetBw());
        EXPECT_EQ(42.0, dp.GetLatency());
    }
}

TEST(DataPath, UnidirectionalDataPath)
{
    Component a, b;
    DataPath dp{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

    ASSERT_NE(nullptr, a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    ASSERT_NE(nullptr, a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_TRUE(a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING)->empty());
    EXPECT_EQ((std::vector{&dp}), *a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));

    ASSERT_NE(nullptr, b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    ASSERT_NE(nullptr, b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ((std::vector{&dp}), *b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    EXPECT_TRUE(b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING)->empty());
}

TEST(DataPath, BidirectionalDataPath)
{
    Component a, b;
    DataPath dp{&a, &b, SYS_SAGE_DATAPATH_BIDIRECTIONAL, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

    ASSERT_NE(nullptr, a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    ASSERT_NE(nullptr, a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ((std::vector{&dp}), *a.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    EXPECT_EQ((std::vector{&dp}), *a.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));

    ASSERT_NE(nullptr, b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    ASSERT_NE(nullptr, b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ((std::vector{&dp}), *b.GetDataPaths(SYS_SAGE_DATAPATH_INCOMING));
    EXPECT_EQ((std::vector{&dp}), *b.GetDataPaths(SYS_SAGE_DATAPATH_OUTGOING));
}

TEST(DataPath, GetDpByType)
{
    Component a, b;
    DataPath dp1{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL};
    DataPath dp2{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
    DataPath dp3{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
    DataPath dp4{&b, &a, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

    EXPECT_EQ(&dp1, a.GetDpByType(SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ(&dp2, a.GetDpByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ(nullptr, a.GetDpByType(SYS_SAGE_DATAPATH_TYPE_L3CAT, SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ(&dp4, a.GetDpByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING));
    EXPECT_EQ(&dp4, b.GetDpByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING));
    EXPECT_EQ(&dp4, b.GetDpByType(SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING | SYS_SAGE_DATAPATH_OUTGOING));
}

TEST(DataPath, GetAllDpByType)
{
    Component a, b;
    DataPath dp1{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_LOGICAL};
    DataPath dp2{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
    DataPath dp3{&a, &b, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};
    DataPath dp4{&b, &a, SYS_SAGE_DATAPATH_ORIENTED, SYS_SAGE_DATAPATH_TYPE_PHYSICAL};

    {
        std::vector<DataPath *> v;
        a.GetAllDpByType(&v, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_INCOMING);
        EXPECT_TRUE(v.empty());
    }

    {
        std::vector<DataPath *> v;
        a.GetAllDpByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING);
        EXPECT_EQ((std::vector{&dp4}), v);
    }

    {
        std::vector<DataPath *> v;
        a.GetAllDpByType(&v, SYS_SAGE_DATAPATH_TYPE_LOGICAL, SYS_SAGE_DATAPATH_OUTGOING);
        EXPECT_EQ((std::vector{&dp1}), v);
    }

    {
        std::vector<DataPath *> v;
        a.GetAllDpByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_OUTGOING);
        EXPECT_EQ((std::vector{&dp2, &dp3}), v);
    }

    {
        std::vector<DataPath *> v;
        a.GetAllDpByType(&v, SYS_SAGE_DATAPATH_TYPE_PHYSICAL, SYS_SAGE_DATAPATH_INCOMING | SYS_SAGE_DATAPATH_OUTGOING);
        EXPECT_EQ((std::vector{&dp2, &dp3, &dp4}), v);
    }
}
