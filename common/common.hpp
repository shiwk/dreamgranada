#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>

#define MAKE_SHARED_PTR(TYPE) using TYPE##Ptr = std::shared_ptr<TYPE>;

#define MAKE_SHARED_PTR_ALIAS_1(TYPE) template<typename T> using TYPE##Ptr = std::shared_ptr<TYPE<T>>;

#define MAKE_WEAK_PTR(TYPE) using TYPE##WeakPtr = std::weak_ptr<TYPE>;

#define FORMAT_STRING(fmt_str, ...) \
    fmt::format(fmt_str, __VA_ARGS__)

#define FORMAT_DOUBLE(d, precision) \
    fmt::format("{:." #precision "f}", d)

using ll = long long;

#endif
