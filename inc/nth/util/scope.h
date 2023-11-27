#ifndef NTH_UTIL_SCOPE_H
#define NTH_UTIL_SCOPE_H

#include <concepts>
#include <tuple>

namespace nth {

/**
 * @brief Specialized utility for executing callables upon scope exit.
 *
 * @tparam Dtor Type of the callable to be executed.
 * @tparam Args Types of arguments to be passed to the callable.
 */
template<class Dtor, class... Args>
struct scope_guard;

/**
 * @brief Primary template for `nth::scope_guard` (no arguments). This 
 * version takes only a callable and ensures that it is executed when 
 * the `nth::scope_guard` goes out of scope. Since it takes no arguments, 
 * there is no memory overhead for empty `std::tuple`. It cannot be
 * dismissed / invoked prematurely, unlike `nth::fused_guard`, but
 * doesn't have overhead for boolean flag.
 *
 * @tparam Dtor Type of the callable to be executed.
 */
template<class Dtor>
struct scope_guard<Dtor> {
    static_assert(std::invocable<Dtor>, "Dtor must be invocable");
    constexpr scope_guard(Dtor&& dtor) : dtor(std::forward<Dtor>(dtor)) {}
    constexpr ~scope_guard() noexcept(std::is_nothrow_invocable_v<Dtor>) 
    { 
        dtor(); 
    }
    constexpr scope_guard(const scope_guard&) = delete;
    constexpr scope_guard& operator=(const scope_guard&) = delete;
    constexpr scope_guard(scope_guard&&) noexcept = default;
    constexpr scope_guard& operator=(scope_guard&&) noexcept = default;
private:
    Dtor dtor;
};

/**
 * @brief Specialized template for `nth::scope_guard` (with arguments).
 * This version takes a callable and its arguments and ensures that the 
 * callable is executed with the provided arguments when the `nth::scope_guard` 
 * goes out of scope. It cannot be dismissed / invoked prematurely, unlike 
 * `nth::fused_guard`, but doesn't have overhead for boolean flag.
 *
 * @tparam Dtor Type of the callable to be executed.
 * @tparam Args Types of arguments to be passed to the callable.
 */
template<class Dtor, class... Args>
struct scope_guard {
    static_assert(std::invocable<Dtor, Args...>, "Dtor must be invocable with Args...");
    constexpr scope_guard(Dtor&& dtor, Args&&... args) : 
        dtor(std::forward<Dtor>(dtor)), 
        args(std::forward<Args>(args)...)
    {}
    constexpr ~scope_guard() noexcept(std::is_nothrow_invocable_v<Dtor, Args...>)
    { 
        std::apply(dtor, args); 
    }
    constexpr scope_guard(const scope_guard&) = delete;
    constexpr scope_guard& operator=(const scope_guard&) = delete;
    constexpr scope_guard(scope_guard&&) noexcept = default;
    constexpr scope_guard& operator=(scope_guard&&) noexcept = default;
private:
    Dtor dtor;
    std::tuple<Args...> args;
};

/**
 * @brief A guard that optionally invokes a callable with given arguments when 
 * it goes out of scope. The invocation can be prevented by calling `defuse()`. 
 * The callable can also be invoked prematurely using `invoke()`, after which 
 * it won't be invoked again upon destruction. Note: There's a slight overhead 
 * compared to `nth::scope_guard` due to the inclusion of a boolean flag to 
 * track the guard's state.
 * 
 * @tparam Dtor Type of the callable to be executed.
 * @tparam Args Types of arguments to be passed to the callable.
 */
template<class Dtor, class... Args>
struct fused_guard {

    static_assert(std::invocable<Dtor, Args...>, "Dtor must be invocable with Args...");

    constexpr fused_guard(Dtor&& dtor, Args&&... args) : 
        dtor(std::forward<Dtor>(dtor)), 
        args(std::forward<Args>(args)...)
    {}
    constexpr ~fused_guard() noexcept(std::is_nothrow_invocable_v<Dtor, Args...>)
    {
        if (done == false)
            std::apply(dtor, args);
    }
    constexpr void invoke() noexcept(std::is_nothrow_invocable_v<Dtor, Args...>)
    {
        if (done == false) {
            done = true;
            std::apply(dtor, args);
        }
    }
    constexpr void defuse() noexcept
    {
        done = true;
    }
    constexpr bool active() const noexcept
    {
        return !done;
    }
    constexpr fused_guard(const fused_guard&) = delete;
    constexpr fused_guard& operator=(const fused_guard&) = delete;
    constexpr fused_guard(fused_guard&&) noexcept = default;
    constexpr fused_guard& operator=(fused_guard&&) noexcept = default;
private:
    Dtor dtor;
    std::tuple<Args...> args;
    bool done = false;
};

}

#endif