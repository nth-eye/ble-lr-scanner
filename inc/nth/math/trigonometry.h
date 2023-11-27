#ifndef NTH_MATH_TRIGONOMETRY_H
#define NTH_MATH_TRIGONOMETRY_H

#include <numbers>
#include <cmath>

namespace nth {

/**
 * @brief Convert decimal degrees to radians.
 * 
 * @tparam T Floating type
 * @param deg Decimal degrees
 * @return Radians
 */
template<std::floating_point T = double>
constexpr T radians(T deg)
{
    return deg * std::numbers::pi_v<T> / T(180.0);
}

/**
 * @brief Convert radians to decimal degrees.
 * 
 * @tparam T Floating type
 * @param rad Radians
 * @return Decimal degrees
 */
template<std::floating_point T = double>
constexpr T degrees(T rad)
{
    return rad * T(180.0) / std::numbers::pi_v<T>;
}

/**
 * @brief Calculate distance in meters on a sphere surface between 
 * two decimal degree points.
 * 
 * @tparam T Floating type
 * @param lat_1 Latitude of first point
 * @param lng_1 Longitude of first point
 * @param lat_2 Latitude of second point
 * @param lng_2 Longitude of second point
 * @param radius Sphere radius in meters
 * @return Distance in meters 
 */
template<std::floating_point T>
inline T haversine(T lat_1, T lng_1, T lat_2, T lng_2, T radius)
{
    T a = pow(sin(radians(lat_2 - lat_1) / T(2)), T(2)) +
          pow(sin(radians(lng_2 - lng_1) / T(2)), T(2)) *
          cos(radians(lat_1)) * 
          cos(radians(lat_2));
    T c = asin(sqrt(a)) * T(2);
    return c * radius;
}

/**
 * @brief Calculate distance in meters on Earth between two 
 * decimal degree points in geographic coordinate system.
 * 
 * @tparam T Floating type
 * @param lat_1 Latitude of first point
 * @param lng_1 Longitude of first point
 * @param lat_2 Latitude of second point
 * @param lng_2 Longitude of second point
 * @return Distance in meters 
 */
template<std::floating_point T = double>
inline T gcs_distance(T lat_1, T lng_1, T lat_2, T lng_2)
{
    return haversine<T>(lat_1, lng_1, lat_2, lng_2, T(6371000));
}

/**
 * @brief Calculate inclination using accelerometer data.
 * 
 * @tparam T Floating type
 * @param x Axis X acceleration 
 * @param y Axis Y acceleration
 * @param z Axis Z acceleration
 * @return Inclination angle in radians
 */
template<std::floating_point T = double>
inline T tilt(T x, T y, T z)
{
    return acos(z / sqrt(x * x + y * y + z * z));
}

/**
 * @brief Calculate Rxyz roll using accelerometer data.
 * 
 * @tparam T Floating type
 * @param y Axis Y acceleration
 * @param z Axis Z acceleration
 * @return Roll angle in radians
 */
template<std::floating_point T = double>
inline T roll(T y, T z)
{
    return atan2(y, z);
}

/**
 * @brief Calculate Rxyz pitch using accelerometer data.
 * 
 * @tparam T Floating type
 * @param x Axis X acceleration
 * @param y Axis Y acceleration
 * @param z Axis Z acceleration
 * @return Pitch angle in radians
 */
template<std::floating_point T = double>
inline T pitch(T x, T y, T z)
{
    return atan2(-x, hypot(y, z)); // sqrt(y * y + z * z)
}

namespace literals {
constexpr auto operator"" _rad(long double deg) { return radians(deg); }
constexpr auto operator"" _deg(long double rad) { return degrees(rad); }
}

}

#endif