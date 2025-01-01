#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>

#define MAKE_SHARED_PTR(TYPE) using TYPE##Ptr = std::shared_ptr<TYPE>;
#define MAKE_WEAK_PTR(TYPE) using TYPE##WeakPtr = std::weak_ptr<TYPE>;
using ll = long long;

#endif
