#include "test.h"
#include "nth/container/vector.h"

namespace nth {
namespace {

constexpr size_t test_size = 5;

using vector_t = vector<objcounter, test_size>;

void verify_vec(const vector_t& obj, size_t copy, size_t move, std::initializer_list<int> arr)
{
    size_t size = arr.size();

    ASSERT_LE(size, test_size);
    ASSERT_EQ(obj.capacity(), test_size);
    ASSERT_EQ(obj.max_size(), test_size);
    ASSERT_EQ(obj.size(), size);
    ASSERT_EQ(obj.full(), size == test_size);
    ASSERT_EQ(obj.empty(), size == 0);
    ASSERT_EQ(obj.begin() + size, obj.end());
    ASSERT_EQ(obj.begin(), obj.data());
    ASSERT_EQ(obj.rbegin() + size, obj.rend());
    ASSERT_EQ(obj.rbegin(), std::reverse_iterator{obj.end()});

    if (obj.empty() == false) {
        ASSERT_EQ(&obj.front(), obj.begin());
        ASSERT_EQ(&obj.back(), obj.end() - 1);
    } else {
        ASSERT_EQ(obj.begin(), obj.end());
        ASSERT_EQ(obj.rbegin(), obj.rend());
    }
    objcounter::verify_count(copy, move);

    for (size_t i = 0; auto it : arr)
        ASSERT_EQ(obj[i++](), it) << "at index " << i;
}

struct ContainerVector : ContainerTester {
    void TearDown() override 
    {
        v.clear();
        ContainerTester::TearDown();
    }
    void setup(std::initializer_list<int> arr)
    {
        ASSERT_LE(arr.size(), test_size);
        v.clear();
        for (auto it : arr)
            v.emplace_back(it);
    }
    void verify(size_t copy, size_t move, std::initializer_list<int> arr)
    {
        verify_vec(v, copy, move, arr);
    }
    vector_t v;
};

TEST_F(ContainerVector, ConstructorDefault)
{
    vector_t x;
    verify_vec(x, 0, 0, {});
}

TEST_F(ContainerVector, ConstructorDefaultN)
{
    vector_t x(4);
    verify_vec(x, 0, 0, {0, 0, 0, 0});
}

TEST_F(ContainerVector, ConstructorConstReferenceN)
{
    vector_t x(4, 77);
    verify_vec(x, 4, 0, {77, 77, 77, 77});
}

TEST_F(ContainerVector, ConstructorRange)
{
    setup({22, 33, 44});
    vector_t x(v.begin(), v.begin() + 2);
    verify_vec(x, 2, 0, {22, 33});
    verify_vec(v, 2, 0, {22, 33, 44}); // verify source didn't change
}

TEST_F(ContainerVector, CopyConstructor)
{
    setup({66, 66, 66});
    vector_t x = v;
    verify_vec(x, 3, 0, {66, 66, 66});
    verify_vec(v, 3, 0, {66, 66, 66});
}

TEST_F(ContainerVector, MoveConstructor)
{
    vector_t y(2, 55);
    vector_t x = std::move(y);
    verify_vec(x, 2, 2, {55, 55});
}

TEST_F(ContainerVector, ConstructorSpan)
{
    const std::array<objcounter, 0> arr_0 = {};
    vector_t x = {arr_0};
    verify_vec(x, 0, 0, {});

    std::array<objcounter, 2> arr_1 = {100, 200};
    vector_t y = {arr_1};
    verify_vec(y, 2, 0, {100, 200});

    objcounter arr_2[5] = {999};
    vector_t z = {arr_2};
    verify_vec(z, 7, 0, {999, 0, 0, 0, 0});
}

TEST_F(ContainerVector, ConstructorArray)
{
    vector_t x = {{1, 1}};
    verify_vec(x, 0, 2, {1, 1});

    vector_t y = {{3, 2, 1, 0, -1}};
    verify_vec(y, 0, 7, {3, 2, 1, 0, -1});
}

TEST_F(ContainerVector, AssignmentOperatorCopy)
{
    const vector_t src(4, 4);
    v = src;
    verify(8, 0, {4, 4, 4, 4});
}

TEST_F(ContainerVector, AssignmentOperatorMove)
{
    v = vector_t(3, 3); 
    verify(3, 3, {3, 3, 3});
}

TEST_F(ContainerVector, AssignmentOperatorSpan)
{
    objcounter arr_0[2] = {1, 2};
    v = arr_0;
    verify(2, 0, {1, 2});

    std::array<objcounter, 5> arr_1 = {5, 4, 3, 2, 1};
    v = arr_1;
    verify(7, 0, {5, 4, 3, 2, 1});

    const std::array<objcounter, 0> arr_2 = {};
    v = arr_2;
    verify(7, 0, {});
}

TEST_F(ContainerVector, AssignmentOperatorArray)
{
    v = {666, 1, 2, 3, 4};
    verify(0, 5, {666, 1, 2, 3, 4});

    v = {objcounter(1), 2};
    verify(0, 7, {1, 2});

    v = {objcounter(42)};
    verify(0, 8, {42});
}

TEST_F(ContainerVector, AssignN)
{
    v.assign(2, 2);
    verify(2, 0, {2, 2});

    v.assign(5, 5);
    verify(7, 0, {5, 5, 5, 5, 5});

    v.assign(0, 0);
    verify(7, 0, {});
}

TEST_F(ContainerVector, AssignRange)
{
    std::array<objcounter, test_size> arr = {1, 2, 3, 4, 5};

    v.assign(arr.begin(), arr.begin() + 2);
    verify(2, 0, {1, 2});

    v.assign(arr.begin(), arr.end());
    verify(7, 0, {1, 2, 3, 4, 5});

    v.assign(arr.begin(), arr.begin());
    verify(7, 0, {});
}

TEST_F(ContainerVector, AssignSpan)
{
    objcounter arr_0[2] = {1, 2};
    v.assign(arr_0);
    verify(2, 0, {1, 2});

    std::array<objcounter, 5> arr_1 = {5, 4, 3, 2, 1};
    v.assign(arr_1);
    verify(7, 0, {5, 4, 3, 2, 1});

    const std::array<objcounter, 0> arr_2 = {};
    v.assign(arr_2);
    verify(7, 0, {});
}

TEST_F(ContainerVector, AssignArray)
{
    objcounter arr[1] = {666};

    v.assign(std::move(arr));
    verify(0, 1, {666});

    v.assign({1, 2});
    verify(0, 3, {1, 2});

    v.assign({5, 4, 3, 2, 1});
    verify(0, 8, {5, 4, 3, 2, 1});
}

TEST_F(ContainerVector, ResizeDefault)
{
    v.resize(2);
    verify(0, 0, {0, 0});

    v.resize(5);
    verify(0, 0, {0, 0, 0, 0, 0});

    v.resize(0);
    verify(0, 0, {});
}

TEST_F(ContainerVector, ResizeReference)
{
    v.resize(2, 0);
    verify(2, 0, {0, 0});

    v.resize(3, 3);
    verify(3, 0, {0, 0, 3});

    v.resize(5, 5);
    verify(5, 0, {0, 0, 3, 5, 5});

    v.resize(0, {});
    verify(5, 0, {});
}

TEST_F(ContainerVector, Clear)
{
    v.resize(5);
    v.clear();
    verify(0, 0, {});
}

TEST_F(ContainerVector, PushBack)
{
    const objcounter obj = 1;

    v.push_back(obj);
    v.push_back(2);
    v.push_back({});

    verify(1, 2, {1, 2, 0});
}

TEST_F(ContainerVector, EmplaceBack)
{
    objcounter obj = 10;

    v.emplace_back(obj);
    v.emplace_back(std::move(obj));
    v.emplace_back(13);
    v.emplace_back();

    verify(1, 1, {10, 10, 13, 0});
}

TEST_F(ContainerVector, Emplace)
{
    setup({1, 2});

    v.emplace(v.begin() + 1, 3);
    verify(0, 1, {1, 3, 2});
    v.emplace(v.end(), 4);
    verify(0, 1, {1, 3, 2, 4});
    v.emplace(v.begin(), 5);
    verify(0, 5, {5, 1, 3, 2, 4});
}

TEST_F(ContainerVector, InsertConstReference)
{
    const objcounter obj = 10;

    setup({1, 2});

    v.insert(v.begin() + 1, obj);
    verify(1, 1, {1, 10, 2});
    v.insert(v.end(), obj);
    verify(2, 1, {1, 10, 2, 10});
    v.insert(v.begin(), obj);
    verify(3, 5, {10, 1, 10, 2, 10});
}

TEST_F(ContainerVector, InsertUniversalReference)
{
    setup({1, 2});

    v.insert(v.begin() + 1, 10);
    verify(0, 2, {1, 10, 2});
    v.insert(v.end(), 10);
    verify(0, 3, {1, 10, 2, 10});
    v.insert(v.begin(), 10);
    verify(0, 8, {10, 1, 10, 2, 10});
}

TEST_F(ContainerVector, InsertN)
{
    const objcounter obj = 10;

    setup({1, 2, 3});
    v.insert(v.begin() + 1, 2, obj);
    verify(2, 2, {1, 10, 10, 2, 3});

    setup({1, 2, 3});
    v.insert(v.begin() + 2, 2, obj);
    verify(4, 3, {1, 2, 10, 10, 3});

    setup({1, 2, 3});
    v.insert(v.end(), 2, obj);
    verify(6, 3, {1, 2, 3, 10, 10});

    setup({1, 2, 3});
    v.insert(v.begin(), 2, obj);
    verify(8, 6, {10, 10, 1, 2, 3});
}

TEST_F(ContainerVector, InsertRange)
{
    std::array<objcounter, 2> arr = {10, 11}; 

    setup({1, 2, 3});
    v.insert(v.begin() + 1, arr.begin(), arr.end());
    verify(2, 2, {1, 10, 11, 2, 3});

    setup({1, 2, 3});
    v.insert(v.begin() + 2, arr.begin(), arr.end());
    verify(4, 3, {1, 2, 10, 11, 3});

    setup({1, 2, 3});
    v.insert(v.end(), arr.begin(), arr.end());
    verify(6, 3, {1, 2, 3, 10, 11});

    setup({1, 2, 3});
    v.insert(v.begin(), arr.begin(), arr.end());
    verify(8, 6, {10, 11, 1, 2, 3});

    setup({1, 2, 3});
    v.insert(v.begin(), arr.begin(), arr.begin());
    verify(8, 6, {1, 2, 3});
}

TEST_F(ContainerVector, InsertIlist)
{
    setup({1, 2, 3});
    v.insert(v.begin() + 1, {10, 11});
    verify(2, 2, {1, 10, 11, 2, 3});

    setup({1, 2, 3});
    v.insert(v.begin() + 2, {10, 11});
    verify(4, 3, {1, 2, 10, 11, 3});

    setup({1, 2, 3});
    v.insert(v.end(), {10, 11});
    verify(6, 3, {1, 2, 3, 10, 11});

    setup({1, 2, 3});
    v.insert(v.begin(), {10, 11});
    verify(8, 6, {10, 11, 1, 2, 3});
}

TEST_F(ContainerVector, PopBack)
{
    setup({1, 2, 3});
    v.pop_back();
    verify(0, 0, {1, 2});
    v.pop_back();
    verify(0, 0, {1});
}

TEST_F(ContainerVector, PopIdx)
{
    setup({1, 2, 3, 4, 5});
    v.pop_idx(0);
    verify(0, 1, {5, 2, 3, 4});
    v.pop_idx(1);
    verify(0, 2, {5, 4, 3});
    v.pop_idx(2);
    verify(0, 2, {5, 4});
}

TEST_F(ContainerVector, PopSingle)
{
    setup({1, 2, 3, 4, 5});
    v.pop(v.begin());
    verify(0, 1, {5, 2, 3, 4});
    v.pop(v.begin() + 1);
    verify(0, 2, {5, 4, 3});
    v.pop(v.begin() + 2);
    verify(0, 2, {5, 4});
}

TEST_F(ContainerVector, PopRange)
{
    setup({1, 2, 3, 4, 5});
    v.pop(v.begin(), v.begin() + 2);
    verify(0, 2, {4, 5, 3});

    setup({1, 2, 3, 4, 5});
    v.pop(v.begin() + 3, v.begin() + 5);
    verify(0, 2, {1, 2, 3});

    setup({1, 2, 3, 4, 5});
    v.pop(v.begin() + 1, v.begin() + 3);
    verify(0, 4, {1, 4, 5});

    setup({1, 2, 3, 4, 5});
    v.pop(v.begin() + 2, v.begin() + 4);
    verify(0, 5, {1, 2, 5});
}

TEST_F(ContainerVector, EraseSingle)
{
    setup({1, 2, 3, 4, 5});
    v.erase(v.begin());
    verify(0, 4, {2, 3, 4, 5});

    setup({1, 2, 3, 4, 5});
    v.erase(v.end() - 1);
    verify(0, 4, {1, 2, 3, 4});

    setup({1, 2, 3, 4, 5});
    v.erase(v.end() - 3);
    verify(0, 6, {1, 2, 4, 5});
}

TEST_F(ContainerVector, EraseRange)
{
    setup({1, 2, 3, 4, 5});
    v.erase(v.begin(), v.begin() + 2);
    verify(0, 3, {3, 4, 5});

    setup({1, 2, 3, 4, 5});
    v.erase(v.end() - 2, v.end());
    verify(0, 3, {1, 2, 3});

    setup({1, 2, 3, 4, 5});
    v.erase(v.begin() + 1, v.begin() + 3);
    verify(0, 5, {1, 4, 5});

    setup({1, 2, 3, 4, 5});
    v.erase(v.begin() + 2, v.begin() + 4);
    verify(0, 6, {1, 2, 5});
}

TEST_F(ContainerVector, Swap)
{
    vector_t x = {{1, 1}};
    verify_vec(x, 0, 2, {1, 1});

    vector_t y = {{3, 2, 1, 0, -1}};
    verify_vec(y, 0, 7, {3, 2, 1, 0, -1});

    x.swap(y);

    verify_vec(x, 0, 19, {3, 2, 1, 0, -1});
    verify_vec(y, 0, 19, {1, 1});
}

TEST_F(ContainerVector, Death)
{
    std::array<objcounter, test_size + 1> arr;

    ASSERT_DEATH(vector_t(test_size + 1), "");
    ASSERT_DEATH(vector_t(test_size + 1, 42), "");
    ASSERT_DEATH(vector_t(arr.begin(), arr.end()), "");
    ASSERT_DEATH(vector_t(std::span{arr}), "");
    ASSERT_DEATH({v = std::span{arr};}, "");
    ASSERT_DEATH(v.assign(v.capacity() + 1, 42), "");
    ASSERT_DEATH(v.assign(arr.begin(), arr.end()), "");
    ASSERT_DEATH(v.assign(std::span{arr}), "");

    ASSERT_DEATH(v[test_size], "");
    ASSERT_DEATH(v.front(), "");
    ASSERT_DEATH(v.back(), "");
    ASSERT_DEATH(v.resize(v.capacity() + 1), "");
    ASSERT_DEATH(v.resize(v.capacity() + 1, 42), "");
    ASSERT_DEATH(v.emplace(v.begin() - 1), "");
    ASSERT_DEATH(v.emplace(v.end() + 1), "");
    ASSERT_DEATH(v.insert(v.begin() - 1, 42), "");
    ASSERT_DEATH(v.insert(v.begin() - 1, {42}), "");
    ASSERT_DEATH(v.insert(v.begin() - 1, 1, objcounter(42)), "");
    ASSERT_DEATH(v.insert(v.begin() - 1, arr.begin(), arr.begin() + 1), "");
    ASSERT_DEATH(v.insert(v.end() + 1, 42), "");
    ASSERT_DEATH(v.insert(v.end() + 1, {42}), "");
    ASSERT_DEATH(v.insert(v.end() + 1, 1, objcounter(42)), "");
    ASSERT_DEATH(v.insert(v.end() + 1, arr.begin(), arr.begin() + 1), "");
    ASSERT_DEATH(v.erase(v.begin()), "");
    ASSERT_DEATH(v.pop_back(), "");
    ASSERT_DEATH(v.pop_idx(0), "");
    ASSERT_DEATH(v.pop(v.begin()), "");

    v.resize(v.capacity());

    ASSERT_DEATH(v.push_back(42), "");
    ASSERT_DEATH(v.emplace_back(42), "");
    ASSERT_DEATH(v.emplace(v.begin()), "");
    ASSERT_DEATH(v.insert(v.begin(), 42), "");
    ASSERT_DEATH(v.insert(v.begin(), {42}), "");
    ASSERT_DEATH(v.insert(v.begin(), 1, objcounter(42)), "");
    ASSERT_DEATH(v.insert(v.begin(), arr.begin(), arr.begin() + 1), "");
    ASSERT_DEATH(v.erase(v.begin() - 1), "");
    ASSERT_DEATH(v.erase(v.end()), "");
    ASSERT_DEATH(v.erase(v.begin() - 1, v.end()), "");
    ASSERT_DEATH(v.erase(v.begin(), v.end() + 1), "");
    ASSERT_DEATH(v.pop(v.begin() - 1), "");
    ASSERT_DEATH(v.pop(v.end()), "");
    ASSERT_DEATH(v.pop(v.begin() - 1, v.end()), "");
    ASSERT_DEATH(v.pop(v.begin(), v.end() + 1), "");
}

}
}