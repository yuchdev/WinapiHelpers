#pragma once

namespace helpers {

/// @brief is_callable<> concept implementation
/// To be removed in C++17
/// @example: static_assert(IsCallable<Func>, "Func() should be callable")
template<typename T>
struct IsCallable {
private:
    typedef char(&yes)[1];
    typedef char(&no)[2];

    struct Fallback { void operator()(); };
    struct Derived : T, Fallback { };

    template<typename U, U> struct Check;

    template<typename>
    static yes test(...);

    template<typename C>
    static no test(Check<void (Fallback::*)(), &C::operator()>*);

public:
    static const bool value = sizeof(test<Derived>(0)) == sizeof(yes);
};

}
