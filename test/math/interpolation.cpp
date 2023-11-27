#include "test.h"
#include "nth/math/interpolation.h"

namespace nth {
namespace {

template<class T>
void check_interpolation(const auto& map, bool valid, size_t size, T x, std::optional<T> y)
{
    ASSERT_EQ(map.valid(), valid);
    ASSERT_EQ(map.size(), size);
    auto ret = map(x);
    if (y) {
        ASSERT_EQ(bool(ret), true);
        assert_near(*ret, *y, 1e-1);
    } else {
        ASSERT_EQ(bool(ret), false);
    }
}

TEST(MathInterpolation, LinearValid)
{
    linear_interpolation<double, 4> map = {
        {{ 1.0, 2.0, 3.0, 4.0   }},
        {{ 2.0, 4.0, 8.0, 16.0  }},
    };
    check_interpolation<double>(map, true, 4, 1.0, 2.0);
    check_interpolation<double>(map, true, 4, 1.5, 3.0);
    check_interpolation<double>(map, true, 4, 2.0, 4.0);
    check_interpolation<double>(map, true, 4, 2.5, 6.0);
    check_interpolation<double>(map, true, 4, 3.0, 8.0);
    check_interpolation<double>(map, true, 4, 3.5, 12.0);
    check_interpolation<double>(map, true, 4, 4.0, 16.0);
    check_interpolation<double>(map, true, 4, 0.0, 0.0);
    check_interpolation<double>(map, true, 4, -0.5, -1.0);
    check_interpolation<double>(map, true, 4, 5.0, 24.0);
    check_interpolation<double>(map, true, 4, 5.5, 28.0);
}

TEST(MathInterpolation, LinearInvalid)
{
    linear_interpolation<double, 4> map = {
        {{  112,    90,     155,    240     }},
        {{  45.0,   25.0,   20.0,   17.5    }},
    };
    check_interpolation<double>(map, false, 0, 120.0, std::nullopt);
}

TEST(MathInterpolation, MakimaValid)
{
    makima_interpolation<double, 16> map = {
        {{  90,     112,    155,    240,    340,    550,    800,    1000,
            1300,   2500,   5000,   55000,  100000, 115000, 120000, 1000000 }},
        {{  45.0,   25.0,   20.0,   17.5,   15.0,   12.5,   10.0,   8.75,
            7.5,    6.25,   5.0,    3.75,   2.5,    1.25,   0.03,   0.02    }},
    };
    check_interpolation<double>(map, true, 16, 300.0, 15.88);
    check_interpolation<double>(map, true, 16, 89.0, std::nullopt);
    check_interpolation<double>(map, true, 16, 1000001.0, std::nullopt);
}

TEST(MathInterpolation, MakimaInvalid)
{
    makima_interpolation<double, 4> map = {
        {{  112,    90,     155,    240     }},
        {{  45.0,   25.0,   20.0,   17.5    }},
    };
    check_interpolation<double>(map, false, 0, 120.0, std::nullopt);
}

}
}