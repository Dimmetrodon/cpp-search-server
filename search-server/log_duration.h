#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <string_view>

#define PROFILE_CONCAT_INTERNAL(X, Y) X ## Y
#define PROFILE_CONCAT(X, Y) PROFILE_CONCAT_INTERNAL(X, Y)
#define UNIQUE_VAR_NAME_PROFILE PROFILE_CONCAT(profileGuard, __LINE__)
#define LOG_DURATION(x) LogDuration UNIQUE_VAR_NAME_PROFILE(x)
#define LOG_DURATION_STREAM(x, y) LogDuration UNIQUE_VAR_NAME_PROFILE(x, y)

class LogDuration {
public:
    using Clock = std::chrono::steady_clock;

    LogDuration(const std::string& operation_name, std::ostream& stream = std::cerr)
        : operation_name_(operation_name)
        , stream_(stream)
    {
    }

    LogDuration(const std::string_view operation_name, std::ostream& stream = std::cerr)
        : operation_name_(operation_name)
        , stream_(stream)
    {
    }

    ~LogDuration() {
        using namespace std::chrono;
        using namespace std::literals;

        const auto end_time = Clock::now();
        const auto dur = end_time - start_time_;
        stream_ << operation_name_ << ": "s << duration_cast<milliseconds>(dur).count() << " ms"s << std::endl;
    }

private:
    const Clock::time_point start_time_ = Clock::now();
    const std::string operation_name_;
    std::ostream& stream_;
};