#include "test.h"
#include "nth/container/ring.h"

namespace nth {
namespace {

constexpr size_t test_size = 4;

using ring_t = ring<objcounter, test_size>;

void print_ring(const auto& r) 
{
    const auto one = r.array_one();
    const auto two = r.array_two();

    printf("+-------------+\n");
    printf("linear:     %u \n", r.linear());
    printf("plain:      %u \n", r.plain());
    printf("full:       %u \n", r.full());
    printf("head:       %lu \n", r.head);
    printf("tail:       %lu \n", r.tail);
    printf("all:        %lu \n", r.size());
    printf("one:        %lu \n", one.size());
    printf("two:        %lu \n", two.size());

    for (const auto& it : r)
        printf("%d ", it());
    printf("\n");

    for (const auto& it : one)
        printf("%d ", it());
    printf("\n");

    for (const auto& it : two)
        printf("%d ", it());
    printf("\n");

    printf("+-------------+\n");
};

void verify_part(std::span<const objcounter> obj, std::initializer_list<int> arr)
{
    ASSERT_EQ(obj.size(), arr.size());
    for (size_t i = 0; auto it : arr)
        ASSERT_EQ(obj[i++](), it) << "at index " << i;
}

void verify_ring(const ring_t& obj, 
    size_t copy, 
    size_t move, 
    std::initializer_list<int> arr,
    std::initializer_list<int> one = {},
    std::initializer_list<int> two = {})
{
    size_t size = arr.size();

    ASSERT_LE(size, test_size);
    ASSERT_EQ(obj.capacity(), test_size);
    ASSERT_EQ(obj.max_size(), test_size);
    ASSERT_EQ(obj.size(), size);
    ASSERT_EQ(obj.full(), size == test_size);
    ASSERT_EQ(obj.empty(), size == 0);
    ASSERT_EQ(obj.begin() + size, obj.end());
    ASSERT_EQ(obj.rbegin() + size, obj.rend());
    ASSERT_EQ(obj.rbegin(), std::reverse_iterator{obj.end()});

    if (obj.empty() == false) {
        ASSERT_EQ(&obj.front(), std::addressof(*obj.begin()));
        ASSERT_EQ(&obj.back(), std::addressof(*(obj.end() - 1)));
    } else {
        ASSERT_EQ(obj.begin(), obj.end());
        ASSERT_EQ(obj.rbegin(), obj.rend());
    }
    objcounter::verify_count(copy, move);

    for (size_t i = 0; auto it : arr)
        ASSERT_EQ(obj[i++](), it) << "at index " << i;

    // printf("lin %lu \n", obj.linear());
    // printf("one %lu \n", obj.array_one().size());
    // printf("two %lu \n", obj.array_two().size());

    if (obj.linear()) {
        verify_part(obj.array_one(), arr);
        verify_part(obj.array_two(), {});
    } else {
        verify_part(obj.array_one(), one);
        verify_part(obj.array_two(), two);
    }
}

struct ContainerRing : ContainerTester {
    void TearDown() override 
    {
        r.clear();
        ContainerTester::TearDown();
    }
    void setup(std::initializer_list<int> arr)
    {
        objcounter::clear_movecopy();
        ASSERT_LE(arr.size(), test_size);
        r.clear();
        for (auto it : arr)
            r.emplace_back(it);
    }
    void verify(
        size_t copy, 
        size_t move, 
        std::initializer_list<int> arr, 
        std::initializer_list<int> one = {},
        std::initializer_list<int> two = {})
    {
        verify_ring(r, copy, move, arr, one, two);
    }
    ring_t r;
};

TEST_F(ContainerRing, ConstructorDefault)
{
    ring_t x;
    verify_ring(x, 0, 0, {});
}

TEST_F(ContainerRing, ConstructorDefaultN)
{
    ring_t x(4);
    verify_ring(x, 0, 0, {0, 0, 0, 0});
}

TEST_F(ContainerRing, ConstructorConstReferenceN)
{
    ring_t x(4, 77);
    verify_ring(x, 4, 0, {77, 77, 77, 77});
}

TEST_F(ContainerRing, ConstructorRange)
{
    setup({22, 33, 44});
    ring_t x(r.begin(), r.begin() + 2);
    verify_ring(x, 2, 0, {22, 33});
    verify_ring(r, 2, 0, {22, 33, 44}); // verify source didn't change
}

TEST_F(ContainerRing, CopyConstructor)
{
    setup({66, 66, 66});
    ring_t x = r;
    verify_ring(x, 3, 0, {66, 66, 66});
    verify_ring(r, 3, 0, {66, 66, 66});
}

TEST_F(ContainerRing, MoveConstructor)
{
    ring_t y(2, 55);
    ring_t x = std::move(y);
    verify_ring(x, 2, 2, {55, 55});
}

TEST_F(ContainerRing, ConstructorSpan)
{
    const std::array<objcounter, 0> arr_0 = {};
    ring_t x = {arr_0};
    verify_ring(x, 0, 0, {}, {}, {});

    std::array<objcounter, 2> arr_1 = {100, 200};
    ring_t y = {arr_1};
    verify_ring(y, 2, 0, {100, 200});

    objcounter arr_2[4] = {999};
    ring_t z = {arr_2};
    verify_ring(z, 6, 0, {999, 0, 0, 0});
}

TEST_F(ContainerRing, ConstructorArray)
{
    ring_t x = {{1, 1}};
    verify_ring(x, 0, 2, {1, 1});

    ring_t y = {{2, 1, 0, -1}};
    verify_ring(y, 0, 6, {2, 1, 0, -1});
}

TEST_F(ContainerRing, AssignmentOperatorCopy)
{
    const ring_t src(4, 4);
    r = src;
    verify(8, 0, {4, 4, 4, 4});
}

TEST_F(ContainerRing, AssignmentOperatorMove)
{
    r = ring_t(3, 3); 
    verify(3, 3, {3, 3, 3});
}

TEST_F(ContainerRing, AssignmentOperatorSpan)
{
    objcounter arr_0[2] = {1, 2};
    r = arr_0;
    verify(2, 0, {1, 2});

    std::array<objcounter, 4> arr_1 = {4, 3, 2, 1};
    r = arr_1;
    verify(6, 0, {4, 3, 2, 1});

    const std::array<objcounter, 0> arr_2 = {};
    r = arr_2;
    verify(6, 0, {});
}

TEST_F(ContainerRing, AssignmentOperatorArray)
{
    r = {666, 1, 2, 3};
    verify(0, 4, {666, 1, 2, 3});

    r = {objcounter(1), 2};
    verify(0, 6, {1, 2});

    r = {objcounter(42)};
    verify(0, 7, {42});
}

TEST_F(ContainerRing, AssignN)
{
    r.assign(2, 2);
    verify(2, 0, {2, 2});

    r.assign(4, 4);
    verify(6, 0, {4, 4, 4, 4});

    r.assign(0, 0);
    verify(6, 0, {});
}

TEST_F(ContainerRing, AssignRange)
{
    std::array<objcounter, test_size> arr = {1, 2, 3, 4};

    r.assign(arr.begin(), arr.begin() + 2);
    verify(2, 0, {1, 2});

    r.assign(arr.begin(), arr.end());
    verify(6, 0, {1, 2, 3, 4});

    r.assign(arr.begin(), arr.begin());
    verify(6, 0, {});
}

TEST_F(ContainerRing, AssignSpan)
{
    objcounter arr_0[2] = {1, 2};
    r.assign(arr_0);
    verify(2, 0, {1, 2});

    std::array<objcounter, 4> arr_1 = {4, 3, 2, 1};
    r.assign(arr_1);
    verify(6, 0, {4, 3, 2, 1});

    const std::array<objcounter, 0> arr_2 = {};
    r.assign(arr_2);
    verify(6, 0, {});
}

TEST_F(ContainerRing, AssignArray)
{
    objcounter arr[1] = {666};

    r.assign(std::move(arr));
    verify(0, 1, {666});

    r.assign({1, 2});
    verify(0, 3, {1, 2});

    r.assign({4, 3, 2, 1});
    verify(0, 7, {4, 3, 2, 1});
}

TEST_F(ContainerRing, Clear)
{
    r.assign({1, 2});
    r.clear();
    verify(0, 2, {});
}

TEST_F(ContainerRing, EraseBegin)
{
    for (int i = 1; i <= 4; ++i)
        r.put_back(i);
    r.erase_begin(1);
    verify_part(r.array_one(), {2, 3, 4});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 4; ++i)
        r.put_back(i);
    r.erase_begin(4);
    verify_part(r.array_one(), { });
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.erase_begin(1);
    verify_part(r.array_one(), {3, 4});
    verify_part(r.array_two(), {5});
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.erase_begin(3);
    verify_part(r.array_one(), {5});
    verify_part(r.array_two(), { });
}

TEST_F(ContainerRing, EraseEnd)
{
    for (int i = 1; i <= 4; ++i)
        r.put_back(i);
    r.erase_end(1);
    verify_part(r.array_one(), {1, 2, 3});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 4; ++i)
        r.put_back(i);
    r.erase_end(4);
    verify_part(r.array_one(), { });
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.erase_end(1);
    verify_part(r.array_one(), {2, 3, 4});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.erase_end(3);
    verify_part(r.array_one(), {2});
    verify_part(r.array_two(), { });
}

TEST_F(ContainerRing, PopBack)
{
    for (int i = 1; i <= 3; ++i)
        r.put_back(i);
    r.pop_back();
    verify(0, 0, {1, 2});
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.pop_back();
    verify(0, 0, {2, 3, 4});
}

TEST_F(ContainerRing, PopFront)
{
    for (int i = 1; i <= 3; ++i)
        r.put_back(i);
    r.pop_front();
    verify(0, 0, {2, 3});
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    r.pop_front();
    verify(0, 0, {3, 4, 5}, {3, 4}, {5});
}

TEST_F(ContainerRing, PushBack)
{
    const objcounter obj = 1;

    r.push_back(obj);
    r.push_back(2);
    r.push_back({});

    verify(1, 2, {1, 2, 0});
}

TEST_F(ContainerRing, PushFront)
{
    const objcounter obj = 1;

    r.push_front(obj);
    r.push_front(2);
    r.push_front({});

    verify(1, 2, {0, 2, 1});
}

TEST_F(ContainerRing, EmplaceBack)
{
    objcounter obj = 10;

    r.emplace_back(obj);
    r.emplace_back(std::move(obj));
    r.emplace_back(13);
    r.emplace_back();

    verify(1, 1, {10, 10, 13, 0});
}

TEST_F(ContainerRing, EmplaceFront)
{
    objcounter obj = 10;

    r.emplace_front(obj);
    r.emplace_front(std::move(obj));
    r.emplace_front(13);
    r.emplace_front();

    verify(1, 1, {0, 13, 10, 10});
}

TEST_F(ContainerRing, PutBack)
{
    const objcounter obj = 1;

    r.put_back(obj);
    r.put_back(2);
    r.put_back();
    r.put_back(3);
    r.put_back(5);

    verify(1, 0, {2, 0, 3, 5}, {2, 0, 3}, {5});
}

TEST_F(ContainerRing, PutFront)
{
    const objcounter obj = 1;

    r.put_front(obj);
    r.put_front(2);
    r.put_front();
    r.put_front(3);
    r.put_front(5);

    verify(1, 0, {5, 3, 0, 2}, {5}, {3, 0, 2});
}

TEST_F(ContainerRing, YeetBack)
{
    const objcounter obj = 1;

    r.yeet_back(obj);
    r.yeet_back(2);
    r.yeet_back();
    r.yeet_back(3);
    r.yeet_back(5);

    verify(1, 0, {1, 2, 0, 3}, {1, 2, 0}, {3});
}

TEST_F(ContainerRing, YeetFront)
{
    const objcounter obj = 1;

    r.yeet_front(obj);
    r.yeet_front(2);
    r.yeet_front();
    r.yeet_front(3);
    r.yeet_front(5);

    verify(1, 0, {3, 0, 2, 1}, {1}, {3, 0, 2});
}

TEST_F(ContainerRing, GetBack)
{
    objcounter item;

    r.push_back(1);
    r.push_back(2);
    r.push_back(3);

    EXPECT_TRUE(r.get_back(item));
    EXPECT_EQ(item(), 3);
    EXPECT_TRUE(r.get_back(item));
    EXPECT_EQ(item(), 2);
    EXPECT_TRUE(r.get_back(item));
    EXPECT_EQ(item(), 1);
    EXPECT_FALSE(r.get_back(item));
}

TEST_F(ContainerRing, GetFront)
{
    objcounter item;

    r.push_back(1);
    r.push_back(2);
    r.push_back(3);

    EXPECT_TRUE(r.get_front(item));
    EXPECT_EQ(item(), 1);
    EXPECT_TRUE(r.get_front(item));
    EXPECT_EQ(item(), 2);
    EXPECT_TRUE(r.get_front(item));
    EXPECT_EQ(item(), 3);
    EXPECT_FALSE(r.get_front(item));
}

TEST_F(ContainerRing, Rotate)
{
    r.push_back(1);
    r.push_back(2);
    r.push_back(3);
    r.push_back(4);

    r.rotate(r.cbegin() + 2); // Rotate the buffer so that the third element becomes the new beginning

    EXPECT_EQ(3, r.front()());
    EXPECT_EQ(2, r.back()());
    EXPECT_EQ(4, r.size());

    r.rotate(r.cend() - 1); // Rotate the buffer again so that the last element becomes the new beginning

    EXPECT_EQ(2, r.front()());
    EXPECT_EQ(1, r.back()());
    EXPECT_EQ(4, r.size());
}

TEST_F(ContainerRing, Linearize)
{
    for (int i = 1; i <= 2; ++i)
        r.put_back(i);
        
    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {1, 2});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 4; ++i)
        r.put_back(i);

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {1, 2, 3, 4});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 4; ++i)
        r.put_back(i);
    for (int i = 1; i <= 2; ++i)
        r.pop_front();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {3, 4});
    verify_part(r.array_two(), { });
    r.clear();



    for (int i = 1; i <= 7; ++i)
        r.put_back(i);

    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {4});
    verify_part(r.array_two(), {5, 6, 7});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {4, 5, 6, 7});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 6; ++i)
        r.put_back(i);

    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {3, 4});
    verify_part(r.array_two(), {5, 6});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {3, 4, 5, 6});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {2, 3, 4});
    verify_part(r.array_two(), {5});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {2, 3, 4, 5});
    verify_part(r.array_two(), { });
    r.clear();



    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    for (int i = 1; i <= 1; ++i)
        r.pop_front();

    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {3, 4});
    verify_part(r.array_two(), {5});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {3, 4, 5});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 5; ++i)
        r.put_back(i);
    for (int i = 1; i <= 2; ++i)
        r.pop_front();

    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {4});
    verify_part(r.array_two(), {5});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {4, 5});
    verify_part(r.array_two(), { });
    r.clear();

    for (int i = 1; i <= 6; ++i)
        r.put_back(i);
    for (int i = 1; i <= 1; ++i)
        r.pop_front();

    ASSERT_EQ(r.linear(), false);
    verify_part(r.array_one(), {4});
    verify_part(r.array_two(), {5, 6});

    r.linearize();

    ASSERT_EQ(r.linear(), true);
    verify_part(r.array_one(), {4, 5, 6});
    verify_part(r.array_two(), { });
    r.clear();


    r.clear();
    for (int i = 1; i <= 2; ++i)
        r.put_back(i);
    for (int i = 1; i <= 2; ++i)
        r.pop_front();
    ASSERT_EQ(r.linear(), true);
}

TEST_F(ContainerRing, Swap)
{
    ring_t x = {{1, 1}};
    verify_ring(x, 0, 2, {1, 1});

    ring_t y = {{3, 2, 1, 0}};
    verify_ring(y, 0, 6, {3, 2, 1, 0});

    x.swap(y);

    verify_ring(x, 0, 16, {3, 2, 1, 0});
    verify_ring(y, 0, 16, {1, 1});
}

TEST_F(ContainerRing, Death)
{
    std::array<objcounter, test_size + 1> arr;

    ASSERT_DEATH(ring_t(test_size + 1), "");
    ASSERT_DEATH(ring_t(test_size + 1, 42), "");
    ASSERT_DEATH(ring_t(arr.begin(), arr.end()), "");
    ASSERT_DEATH(ring_t(std::span{arr}), "");
    ASSERT_DEATH({r = std::span{arr};}, "");
    ASSERT_DEATH(r.assign(r.capacity() + 1, 42), "");
    ASSERT_DEATH(r.assign(arr.begin(), arr.end()), "");
    ASSERT_DEATH(r.assign(std::span{arr}), "");

    r = {0, 1, 2, 3};
    ring_t x = {{1, 1}};

    ASSERT_DEATH(r.push_front(0), "");
    ASSERT_DEATH(r.push_back(0), "");
    ASSERT_DEATH(r.emplace_front(), "");
    ASSERT_DEATH(r.emplace_back(), "");
    ASSERT_DEATH(r.erase_begin(test_size + 1), "");
    ASSERT_DEATH(r.erase_end(test_size + 1), "");
    ASSERT_DEATH(r.rotate(r.end()), "");
    ASSERT_DEATH(r.rotate(x.begin()), "");
}

}
}