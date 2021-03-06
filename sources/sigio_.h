#pragma once

/* STL C++ headers */
#include <map>
#include <vector>
#include <memory>
#include <ratio>
#include <chrono>
#include <ctime>
#include <csignal>
#include <climits>
#include <system_error>
#include <string>
#include <deque>

/* Linux system headers */
#include <syslog.h>
#include <pthread.h>
//#include <limits.h>


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

    struct inotify_io_
    {
        uint32_t mask;
        inotify_callback_t callback;
    };

    int _signal;
    sigset_t _sig_set;
    pthread_t _sig_thread;

    std::map<int, io_> _io;
    int _inotify_fd;
    std::map<std::string, int> _inotify_wd;
    std::map<int, inotify_io_> _inotify_io;
    std::vector<uint8_t> _inotify_buff;

public:
    static const size_t INOTIFY_BUFF_SIZE = 10*(sizeof(struct inotify_event) + NAME_MAX +1);
    static void* sig_thread(void* arg);
    static void inotify_handler(siginfo_t*);
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

    void activate(const std::string fn, uint32_t inotify_mask, inotify_callback_t cb,
          seconds timeout_sec = 0s,
          nanoseconds timeout_nsec = 0ns);

    void deactivate(int fd);
    void deactivate(const std::string fn);

    bool is_activated(int fd) const noexcept;
    bool is_activated(const std::string fn) const noexcept;

    std::error_code try_activate(int fd, callback_t cb,
         seconds timeout_sec = 0s,
         nanoseconds timeout_nsec = 0ns) noexcept;

    std::error_code try_activate(const std::string fn, uint32_t inotify_mask, inotify_callback_t cb,
         seconds timeout_sec = 0s,
         nanoseconds timeout_nsec = 0ns) noexcept;

    std::error_code try_deactivate(int fd) noexcept;
    std::error_code try_deactivate(const std::string fn) noexcept;
  };
} //namespace posixcpp
