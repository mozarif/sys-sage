#include "raii.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

TEST(RAII, InnerCannotBeNull)
{
    ASSERT_ANY_THROW(RAII(reinterpret_cast<void *>(0), free));
    ASSERT_ANY_THROW(RAII(reinterpret_cast<int *>(0), [](void *) {}));
}

TEST(RAII, DestructorRuns)
{
    int x = 0;
    {
        RAII raii{&x, [](int *x)
                  { *x = 42; }};
        ASSERT_EQ(x, 0);
        ASSERT_EQ(*static_cast<int *>(raii), 0);
    }
    ASSERT_EQ(x, 42);
}

TEST(RAII, MoveOwnership)
{
    int x = 0;
    {
        RAII r0{&x, [](int *x)
                { *x += 1; }};
        RAII r1{std::move(r0)};
        ASSERT_EQ(x, 0);
        ASSERT_EQ(*static_cast<int *>(r1), 0);
    }
    ASSERT_EQ(x, 1);
}
