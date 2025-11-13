#include <gtest/gtest.h>
#include <util.hpp>

TEST(UtilTests, TestLastWeekDayNotLaterThan)
{
    using namespace granada::utils::date;

    for (int mDay = 1; mDay <= 7; ++mDay)
    {
        // 2025-01-01 (Wednesday) to 2025-01-07 (Tuesday)
        std::tm tm = {};
        tm.tm_year = 2025 - 1900;
        tm.tm_mon = 0; // January
        tm.tm_mday = mDay;
        tm.tm_hour = 1;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        time_t test_time = std::mktime(&tm);

        for (int target_weekday = 0; target_weekday <= 6; ++target_weekday)
        {
            for (int h = 0; h < 24; ++h)
            {
                uint64_t result_ts = lastWeekDayNotLaterThan(static_cast<uint64_t>(test_time), target_weekday, h, 0, 0);
                std::tm *result_tm = std::gmtime(reinterpret_cast<time_t *>(&result_ts));
                EXPECT_EQ(result_tm->tm_wday, target_weekday);
                EXPECT_LE(result_ts, static_cast<uint64_t>(test_time)); // <=

                int diff = (tm.tm_wday - target_weekday + 7) % 7;
                if (diff == 0)
                {
                    if (tm.tm_hour < h)
                    {
                        diff = 7;
                    }
                }
                int expected_day = mDay - diff;

                std::tm tmExpected = tm;
                tmExpected.tm_mday = expected_day;
                tmExpected.tm_hour = h;
                time_t expected_time = std::mktime(&tmExpected);
                EXPECT_EQ(static_cast<time_t>(result_ts), expected_time) << "expected tm_day: " << tmExpected.tm_mday << " result tm_day: " << result_tm->tm_mday << " for target_weekday: " << target_weekday << " before " << tm.tm_wday;
            }
        }
    }
}

TEST(UtilTests, TestLastMonthNotLaterThan)
{
    using namespace granada::utils::date;

    for (int mDay = 1; mDay <= 31; ++mDay)
    {
        // 2025-03-01 to 2025-03-31
        std::tm tm = {};
        tm.tm_year = 2025 - 1900;
        tm.tm_mon = 2; // March
        tm.tm_mday = mDay;
        tm.tm_hour = 1;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        time_t test_time = std::mktime(&tm);

        for (int target_monthday = 1; target_monthday <= 31; ++target_monthday)
        {
            for (int h = 0; h < 24; ++h)
            {
                uint64_t result_ts = lastMonthDayNotLaterThan(static_cast<uint64_t>(test_time), target_monthday, h, 0, 0);
                std::tm *result_tm = std::gmtime(reinterpret_cast<time_t *>(&result_ts));
                EXPECT_EQ(result_tm->tm_mday, target_monthday);
                EXPECT_LE(result_ts, static_cast<uint64_t>(test_time)); // <=

                std::tm tmExpected = tm;
                int expected_day = target_monthday;
                tmExpected.tm_mday = expected_day;
                tmExpected.tm_hour = h;
                if (mDay < target_monthday ||
                    (mDay == target_monthday && tm.tm_hour < h))
                {
                    // go to previous month
                    tmExpected.tm_mon = target_monthday > 28 ? 0 : 1;
                }

                time_t expected_time = std::mktime(&tmExpected);
                EXPECT_EQ(static_cast<time_t>(result_ts), expected_time) << "expected month:" << tmExpected.tm_mon << " expected tm_day:" << tmExpected.tm_mday << " result tm_mon:" << result_tm->tm_mon << " result tm_day:" << result_tm->tm_mday << " result tm_hour:" << result_tm->tm_hour << " target_monthday:" << target_monthday << " target tm_hour:" << h << " in month:" << (tm.tm_mon);
            }
        }
    }
}