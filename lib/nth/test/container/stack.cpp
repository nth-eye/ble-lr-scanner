#include "test.h"
#include "nth/container/stack.h"
#include <ranges> 

namespace nth {
namespace {

constexpr size_t test_size = 5;

using stack_t = stack<objcounter, test_size>;

void verify_stack(stack_t& obj, size_t copy, size_t move, std::initializer_list<int> arr)
{
    size_t size = arr.size();

    ASSERT_LE(size, test_size);
    ASSERT_EQ(obj.capacity(), test_size);
    ASSERT_EQ(obj.max_size(), test_size);
    ASSERT_EQ(obj.size(), size);
    ASSERT_EQ(obj.full(), size == test_size);
    ASSERT_EQ(obj.empty(), size == 0);

    objcounter::verify_count(copy, move);

    for (size_t i = 0; auto it : std::views::reverse(arr)) {
        ASSERT_EQ(obj.top()(), it) << "at index " << i;
        obj.pop();
        ASSERT_EQ(obj.size(), size - ++i);
    }
}

struct ContainerStack : ContainerTester {
    void TearDown() override 
    {
        s.clear();
        ContainerTester::TearDown();
    }
    void setup(std::initializer_list<int> arr)
    {
        ASSERT_LE(arr.size(), test_size);
        s.clear();
        for (auto it : arr)
            s.emplace(it);
    }
    void verify(size_t copy, size_t move, std::initializer_list<int> arr)
    {
        verify_stack(s, copy, move, arr);
    }
    stack_t s;
};

TEST_F(ContainerStack, ConstructorDefault)
{
    stack_t x;
    verify_stack(x, 0, 0, {});
}

TEST_F(ContainerStack, ConstructorRange)
{
    std::array<objcounter, 3> a = {22, 33, 44};
    stack_t x(a.begin(), a.begin() + 2);
    verify_stack(x, 2, 0, {22, 33});
}

TEST_F(ContainerStack, CopyConstructor)
{
    setup({66, 66, 66});
    stack_t x = s;
    verify_stack(x, 3, 0, {66, 66, 66});
    verify_stack(s, 3, 0, {66, 66, 66});
}

TEST_F(ContainerStack, MoveConstructor)
{
    stack_t y = {{55, 55}};
    stack_t x = std::move(y);
    verify_stack(x, 0, 4, {55, 55});
}

TEST_F(ContainerStack, ConstructorSpan)
{
    const std::array<objcounter, 0> arr_0 = {};
    stack_t x = {arr_0};
    verify_stack(x, 0, 0, {});

    std::array<objcounter, 2> arr_1 = {100, 200};
    stack_t y = {arr_1};
    verify_stack(y, 2, 0, {100, 200});

    objcounter arr_2[5] = {999};
    stack_t z = {arr_2};
    verify_stack(z, 7, 0, {999, 0, 0, 0, 0});
}

TEST_F(ContainerStack, AssignmentOperatorCopy)
{
    const stack_t src({4, 4, 4, 4});
    s = src;
    verify(4, 4, {4, 4, 4, 4});
}

TEST_F(ContainerStack, AssignmentOperatorMove)
{
    s = stack_t({3, 3, 3}); 
    verify(0, 6, {3, 3, 3});
}

TEST_F(ContainerStack, Clear)
{
    setup({1, 1, 1, 1, 1}); 
    s.clear();
    verify(0, 0, {});
}

TEST_F(ContainerStack, Push)
{
    const objcounter obj = 1;

    s.push(obj);
    s.push(2);
    s.push({});

    verify(1, 2, {1, 2, 0});
}

TEST_F(ContainerStack, Emplace)
{
    objcounter obj = 10;

    s.emplace(obj);
    s.emplace(std::move(obj));
    s.emplace(13);
    s.emplace();

    verify(1, 1, {10, 10, 13, 0});
}

TEST_F(ContainerStack, Pop)
{
    setup({1, 2, 3});
    s.pop();
    verify(0, 0, {1, 2});
    setup({1, 2, 3});
    s.pop();
    s.pop();
    verify(0, 0, {1});
}

TEST_F(ContainerStack, PopGet)
{
    setup({1, 2, 3});
    ASSERT_EQ(s.pop_get()(), 3);
    verify(1, 0, {1, 2});
    setup({1, 2, 3});
    ASSERT_EQ(s.pop_get()(), 3);
    ASSERT_EQ(s.pop_get()(), 2);
    verify(3, 0, {1});
}

TEST_F(ContainerStack, Swap)
{
    stack_t x = {{1, 1}};
    stack_t y = {{3, 2, 1, 0, -1}};

    x.swap(y);

    verify_stack(x, 0, 19, {3, 2, 1, 0, -1});
    verify_stack(y, 0, 19, {1, 1});
}

}
}