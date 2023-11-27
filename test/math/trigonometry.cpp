#include "test.h"
#include "nth/math/trigonometry.h"

namespace nth {
namespace {

TEST(MathTrigonometry, Radians)
{
    assert_near(radians(0.0),        0);
    assert_near(radians(15.0),       M_PI/12);
    assert_near(radians(30.0),       M_PI/6);
    assert_near(radians(45.0),       M_PI/4);
    assert_near(radians(90.0),       M_PI/2);
    assert_near(radians(180.0),      M_PI);
    assert_near(radians(360.0),      M_PI*2);
    assert_near(radians(540.0),      M_PI*3);
    assert_near(radians(66.6),       1.1623892818);

    assert_near(radians(-0.0),       0);
    assert_near(radians(-15.0),     -M_PI/12);
    assert_near(radians(-30.0),     -M_PI/6);
    assert_near(radians(-45.0),     -M_PI/4);
    assert_near(radians(-90.0),     -M_PI/2);
    assert_near(radians(-180.0),    -M_PI);
    assert_near(radians(-360.0),    -M_PI*2);
    assert_near(radians(-540.0),    -M_PI*3);
    assert_near(radians(-66.6),     -1.1623892818);
}

TEST(MathTrigonometry, Degrees)
{
    assert_near(degrees(0.0),        0);
    assert_near(degrees(1.0),        57.2957795131);
    assert_near(degrees(3.14),       179.9087476711);
    assert_near(degrees(M_PI),       180);
    assert_near(degrees(M_PI*2),     360);
    assert_near(degrees(M_PI*3),     540);
    assert_near(degrees(M_PI/4),     45);
    assert_near(degrees(M_PI/2),     90);

    assert_near(degrees(-0.0),        0);
    assert_near(degrees(-1.0),       -57.2957795131);
    assert_near(degrees(-3.14),      -179.9087476711);
    assert_near(degrees(-M_PI),      -180);
    assert_near(degrees(-M_PI*2),    -360);
    assert_near(degrees(-M_PI*3),    -540);
    assert_near(degrees(-M_PI/4),    -45);
    assert_near(degrees(-M_PI/2),    -90);
}

TEST(MathTrigonometry, Haversine)
{
    assert_near(haversine( 42.42,  13.37,  66.66,  77.77, 6371000.0), 4645479,    1);
    assert_near(haversine( 24.42,  99.99, -22.11, -57.93, 6371000.0), 17747388,   1);
    assert_near(haversine(-71.45, -69.69,  76.69,  69.03, 6371000.0), 18664684,   1);
}

TEST(MathTrigonometry, GcsDistance)
{
    assert_near(gcs_distance( 42.42,  13.37,  66.66,  77.77), 4645479,    1);
    assert_near(gcs_distance( 24.42,  99.99, -22.11, -57.93), 17747388,   1);
    assert_near(gcs_distance(-71.45, -69.69,  76.69,  69.03), 18664684,   1);
}

TEST(MathTrigonometry, Tilt)
{
    assert_near(tilt(0.0, 0.0, 1.0), 0.0);          // Test case: x = 0, y = 0, z = 1 (upright)
    assert_near(tilt(1.0, 0.0, 0.0), M_PI / 2.0);   // Test case: x = 1, y = 0, z = 0 (tilted right)
    assert_near(tilt(0.0, 1.0, 0.0), M_PI / 2.0);   // Test case: x = 0, y = 1, z = 0 (tilted forward)
}

TEST(MathTrigonometry, Roll)
{
    assert_near(roll(0.0,   1.0),           0.0);           // Test case: y = 0, z = 1 (upright)
    assert_near(roll(1.0,   0.0),           M_PI / 2.0);    // Test case: y = 1, z = 0 (sideways)
    assert_near(roll(0.0,  -1.0),           M_PI);          // Test case: y = 0, z = -1 (upside down)
    assert_near(roll(0.5,   sin(M_PI / 3)), M_PI / 6);
}

TEST(MathTrigonometry, Pitch)
{

}

}
}