#ifndef NTH_TEST_H
#define NTH_TEST_H

#include <gtest/gtest.h>
#include <cassert>
#include <span>

#undef NDEBUG

struct objtracer {
    objtracer() { 
        puts("default constructor"); 
    }
    objtracer(int y) {
        puts("argument constructor");
        x = y;
    }
    ~objtracer() {
        puts("destructor");
        x = -99;
    }
    objtracer(const objtracer& rhs) noexcept { 
        puts("copy constructor");
        x = rhs.x;
    }
    objtracer(objtracer&& rhs) noexcept { 
        puts("move constructor");
        x = rhs.x;
    }
    objtracer& operator=(const objtracer& rhs) noexcept {
        puts("copy assignment");
        x = rhs.x;
        return *this;
    }
    objtracer& operator=(objtracer&& rhs) noexcept {
        puts("move assignment");
        x = rhs.x;
        return *this;
    }
    int x = 0;
};

struct objcounter {
    objcounter() : self(this), x(0) 
    {
        ++livecnt;
    }
    objcounter(int x) : self(this), x(x) 
    {
        ++livecnt;
    }
    objcounter(const objcounter& other) : self(this), x(other.x)
    {
        ++livecnt;
        ++copycnt;
    }
    objcounter(objcounter&& other) noexcept : self(this), x(std::move(other.x))
    {
        ++livecnt;
        ++movecnt;
    }
    ~objcounter() 
    {
        --livecnt;
        assert(self == this);
    }
    objcounter& operator=(const objcounter& other) 
    {
        ++copycnt;
        x = other.x;
        assert(self == this);
        assert(other.self == &other);
        return *this;
    }
    objcounter& operator=(objcounter&& other) noexcept
    {
        ++movecnt;
        x = std::move(other.x);
        assert(self == this);
        assert(other.self == &other);
        return *this;
    }
    int operator()() const
    {
        return x;
    }
    static size_t livecount()
    {
        return livecnt;
    }
    static size_t movecount()
    {
        return movecnt;
    }
    static size_t copycount()
    {
        return copycnt;
    }
    static void clear()
    {
        livecnt = 0;
        copycnt = 0;
        movecnt = 0;
    }
    static void clear_movecopy()
    {
        copycnt = 0;
        movecnt = 0;
    }
    static void verify_count(size_t copy, size_t move)
    {
        ASSERT_EQ(copycnt, copy);
        ASSERT_EQ(movecnt, move);
    }
private:
    inline static size_t livecnt = 0;
    inline static size_t copycnt = 0;
    inline static size_t movecnt = 0;
    const objcounter* self;
    int x;
};

class ContainerTester : public ::testing::Test {
protected:
    void SetUp() override 
    {
        objcounter::clear();
    }
    void TearDown() override 
    {
        ASSERT_EQ(objcounter::livecount(), 0);
    }
};

inline void assert_near(double a, double b, double precision = 1e-10)
{
    ASSERT_NEAR(a, b, precision);
}

template<class T>
inline void assert_arreq(const T* lhs, const T* rhs, size_t len)
{
    ASSERT_TRUE(lhs);
    ASSERT_TRUE(rhs);
    for (size_t i = 0; i < len; ++i)
        ASSERT_EQ(lhs[i], rhs[i]) << "at index " << i;
}

inline void assert_arreq(std::span<const uint8_t> lhs, std::span<const uint8_t> rhs)
{
    ASSERT_EQ(lhs.size(), rhs.size());
    for (size_t i = 0; i < rhs.size(); ++i)
        ASSERT_EQ(lhs[i], rhs[i]) << "at index " << i;
}

// inline void assert_streq(const char* lhs, const char* rhs)
// {
//     ASSERT_TRUE(lhs);
//     ASSERT_TRUE(rhs);
//     ASSERT_TRUE(strlen(lhs) == strlen(rhs));
//     for (size_t i = 0; i < strlen(lhs); ++i)
//         ASSERT_EQ(lhs[i], rhs[i]) << "at index " << i;
// }

#endif