#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <glog/logging.h>
#include <sstream>
#include <fmt/core.h>


#define LOG_FMT(level, fmt_str, ...) \
    LOG(level) << fmt::format(fmt_str, __VA_ARGS__)

#define LOG_MSG(level, str) \
    LOG(level) << str

#endif