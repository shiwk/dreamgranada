#ifndef COMMON_HPP
#define COMMON_HPP

#include <memory>

// #define MAKE_SHARED_PTR(TYPE)                        \
//     namespace granda                                 \
//     {                                                \
//         namespace ioloop                             \
//         {                                            \
//             using TYPE##Ptr = std::shared_ptr<TYPE>; \
//         }                                            \
//     } \

#define MAKE_SHARED_PTR(TYPE) using TYPE##Ptr = std::shared_ptr<TYPE>;
#endif