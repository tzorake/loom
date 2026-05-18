#ifndef TZOVERLOAD_HPP
#define TZOVERLOAD_HPP

template <typename... Args>
struct TzOverload
{
    // Non-const member function
    template <typename R, typename C>
    static constexpr auto of(R (C::*pmf)(Args...)) noexcept -> R (C::*)(Args...)
    {
        return pmf;
    }

    // Free / static function
    template <typename R>
    static constexpr auto of(R (*pf)(Args...)) noexcept -> R (*)(Args...)
    {
        return pf;
    }
};

template <typename... Args>
struct TzConstOverload
{
    // Const member function
    template <typename R, typename C>
    static constexpr auto of(R (C::*pmf)(Args...) const) noexcept -> R (C::*)(Args...) const
    {
        return pmf;
    }
};

template <typename... Args>
struct TzNoexceptOverload
{
    // Noexcept non-const member function
    template <typename R, typename C>
    static constexpr auto of(R (C::*pmf)(Args...) noexcept) noexcept -> R (C::*)(Args...) noexcept
    {
        return pmf;
    }

    // Noexcept free / static function
    template <typename R>
    static constexpr auto of(R (*pf)(Args...) noexcept) noexcept -> R (*)(Args...) noexcept
    {
        return pf;
    }
};

template <typename... Args>
struct TzNoexceptConstOverload
{
    // Noexcept const member function
    template <typename R, typename C>
    static constexpr auto of(R (C::*pmf)(Args...) const noexcept) noexcept
        -> R (C::*)(Args...) const noexcept
    {
        return pmf;
    }
};

template <typename... Args, typename R, typename C>
constexpr auto tzOverload(R (C::*pmf)(Args...)) noexcept -> R (C::*)(Args...)
{
    return pmf;
}

template <typename... Args, typename R>
constexpr auto tzOverload(R (*pf)(Args...)) noexcept -> R (*)(Args...)
{
    return pf;
}

template <typename... Args, typename R, typename C>
constexpr auto tzConstOverload(R (C::*pmf)(Args...) const) noexcept -> R (C::*)(Args...) const
{
    return pmf;
}

template <typename... Args, typename R, typename C>
constexpr auto tzNoexceptOverload(R (C::*pmf)(Args...) noexcept) noexcept
    -> R (C::*)(Args...) noexcept
{
    return pmf;
}

template <typename... Args, typename R>
constexpr auto tzNoexceptOverload(R (*pf)(Args...) noexcept) noexcept -> R (*)(Args...) noexcept
{
    return pf;
}

template <typename... Args, typename R, typename C>
constexpr auto tzNoexceptConstOverload(R (C::*pmf)(Args...) const noexcept) noexcept
    -> R (C::*)(Args...) const noexcept
{
    return pmf;
}

#endif // TZOVERLOAD_HPP
