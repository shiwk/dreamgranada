#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <glog/logging.h>
#include <sstream>
#include <fmt/core.h>
#include "common.hpp"

#define LOG_FMT(level, fmt_str, ...) \
    LOG(level) << fmt::format(fmt_str, __VA_ARGS__)


#define LOG_MSG(level, str) \
    LOG(level) << str

#define LOG_DEBUG(msg) \
    DLOG(INFO) << msg

#define LOG_INFO(msg) \
    LOG_MSG(INFO, msg)

#define LOG_ERROR(msg) \
    LOG_MSG(ERROR, msg)

#define LOG_WARNING(msg) \
    LOG_MSG(WARNING, msg)

#define LOG_FATAL(msg) \
    LOG_MSG(FATAL, msg)

#define LOG_DEBUG_FMT(fmt_str, ...) \
    DLOG(INFO) << fmt::format(fmt_str, __VA_ARGS__)

#define LOG_INFO_FMT(fmt_str, ...) \
    LOG_FMT(INFO, fmt_str, __VA_ARGS__)

#define LOG_ERROR_FMT(fmt_str, ...) \
    LOG_FMT(ERROR, fmt_str, __VA_ARGS__)

#define LOG_WARNING_FMT(fmt_str, ...) \
    LOG_FMT(WARNING, fmt_str, __VA_ARGS__)

#define LOG_FATAL_FMT(fmt_str, ...) \
    LOG_FMT(FATAL, fmt_str, __VA_ARGS__)


#endif