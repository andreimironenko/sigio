#pragma once

/* STL C++ headers */
#include <map>
#include <memory>
#include <ratio>
#include <chrono>
#include <ctime>
#include <system_error>

/* Linux system headers */
#include <syslog.h>

/* Local headers */
#include "timer.h"

namespace posixcpp
{
  class timer::timer_
  {
    std::chrono::seconds _period_sec;
    std::chrono::nanoseconds _period_nsec;
    callback_t _callback;
    void* _data;
    bool _is_single_shot;
    int _signal;

    struct itimerspec _ts;
    struct sigaction _sa;
    struct sigevent _sev;

    timer_t _timer;

    public:
    static void signal_handler(int sig, siginfo_t *si, void *uc = nullptr);

    explicit timer_(std::chrono::seconds period_sec, std::chrono::nanoseconds period_nsec,
        callback_t callback, void* data,
        bool is_single_short, int sig);

    ~timer_();

    timer_(const timer_&) = delete;
    timer_(timer_&&) = delete;
    timer_& operator=(const timer_&) = delete;
    timer_& operator=(timer_&&) = delete;

    void start();
    void reset();
    void suspend();
    void resume();
    void stop();

    std::error_code try_start() noexcept;
    std::error_code try_reset() noexcept;
    std::error_code try_suspend() noexcept;
    std::error_code try_resume() noexcept;
    std::error_code try_stop() noexcept;
  };
} //namespace posixcpp
