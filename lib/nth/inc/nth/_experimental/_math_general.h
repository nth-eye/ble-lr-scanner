// /**
//  * @brief Get floating number length in symbols for given precision.
//  * 
//  * @param x Floating value
//  * @param p Precision
//  * @return Length
//  */
// constexpr size_t flen(std::floating_point auto x, int p = 5)
// {
//     auto whole = static_cast<long long>(x);
//     // x -= whole;
//     auto len = ilen(whole);
//     // x *= ipow();
//     // for (int i = 0; i < p; ++i)
//     //     x *= 10;
//     return len + 1 + p; // ilen(static_cast<long long>(x)); // +1 for dot
// }

// constexpr auto ghb = std::numeric_limits<float>::digits;
// constexpr auto ghd = std::numeric_limits<double>::digits;

// template<class T, size_t N>
// auto long_multiplication_1(
//     const std::array<T, N>& a, 
//     const std::array<T, N>& b) 
// {
//     std::array<T, N * 2> result {};
//     for (size_t i = 0; i < N; ++i) {
//         if (a[i] == 0)
//             continue;
//         T carry = 0;
//         for (size_t j = 0; j < N; ++j) {
//             auto product = 
//                 static_cast<uint64_t>(a[i]) * 
//                 static_cast<uint64_t>(b[j]) + 
//                 static_cast<uint64_t>(result[i + j]) + carry;
//             result[i + j] = static_cast<T>(product);
//             carry = product >> std::numeric_limits<T>::digits;
//         }
//         result[i + N] = carry;
//     }
//     return result;
// }