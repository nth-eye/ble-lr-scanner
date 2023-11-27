#ifndef NTH_UTIL_TYPEID_H
#define NTH_UTIL_TYPEID_H

#include <string_view>

namespace nth {
namespace imp {

/**
 * @brief Get human-readable name of the given type as a string view. 
 * Used internally to fill null-terminated array of chars.
 * 
 * @tparam T Type
 * @return String view representing the name of the type `T`
 */
template<class T>
constexpr auto type_name()
{
#ifdef __clang__
    std::string_view name = __PRETTY_FUNCTION__;
    std::string_view pref = "auto nth::imp::type_name() [T = ";
    std::string_view suff = "]";
#elif defined(__GNUC__)
    std::string_view name = __PRETTY_FUNCTION__;
    std::string_view pref = "constexpr auto nth::imp::type_name() [with T = ";
    std::string_view suff = "]";
#elif defined(_MSC_VER)
    std::string_view name = __FUNCSIG__;
    std::string_view pref = "auto __cdecl nth::imp::type_name<";
    std::string_view suff = ">(void)";
#endif
    name.remove_prefix(pref.size());
    name.remove_suffix(suff.size());
    return name;
};

/**
 * @brief Helper to create class specific static pointer and name.
 * 
 * @tparam T Type
 */
template<class T>
struct type_id {
    static constexpr auto ptr = nullptr;
    static constexpr auto str = [] {
        constexpr auto sv = type_name<T>();
        std::array<char, sv.size() + 1> result = {};
        std::copy(sv.begin(), sv.end(), result.data());
        return result;
    }();
};

}

/**
 * @brief Type ID.
 * 
 */
using tid = const void*;

/**
 * @brief Unique compile-time identificator for a type. Can 
 * be checked for equality and used as template parameter. 
 * 
 * @tparam T Type
 */
template<class T>
inline constexpr tid type_id = &imp::type_id<T>::ptr;

/**
 * @brief Human-readable name of the given type as a string view.
 * Has null-terminator at the end, but isn't counted in size.
 * 
 * @tparam T Type
 */
template<class T>
inline constexpr std::string_view type_name = {
    imp::type_id<T>::str.data(), 
    imp::type_id<T>::str.size() - 1
};

}

#endif