#pragma once

/* STL C++ headers */
#include <map>
#include <memory>
#include <ratio>
#include <chrono>
#include <ctime>
#include <csignal>
#include <system_error>

/* Linux system headers */
#include <syslog.h>

/* Local headers */
#include "sigio.h"

using std::chrono::seconds;
using std::chrono::nanoseconds;

namespace one
{
  class sigio::sigio_
  {
    struct io_
    {
      int _flags;
      callback_t _callback;
      seconds _timeout_sec;
      nanoseconds _timeout_nsec;
    };

    int _signal;
    std::map<int, io_> _io_map;

    public:
    static void sigaction_handler(int sig, siginfo_t *si, void *uc = nullptr);
    static sigio_& get();

    explicit sigio_(int signal = SIGRTMIN);

    ~sigio_();

    sigio_(const sigio_&) = delete;
    sigio_(sigio_&&) = delete;
    sigio_& operator=(const sigio_&) = delete;
    sigio_& operator=(sigio_&&) = delete;

    int get_signal() const;
    std::error_code set_signal(int sig);

    void activate(int fd, callback_t cb,
        seconds timeout_sec = 0s,
        nanoseconds timeout_nsec = 0ns);
    void deactivate(int fd);

    bool is_activated(int fd) const noexcept;

    std::error_code try_activate(int fd, callback_t cb,
         seconds timeout_sec = 0s,
         nanoseconds timeout_nsec = 0ns) noexcept;
    std::error_code try_deactivate(int fd) noexcept;
  };
} //namespace posixcpp
