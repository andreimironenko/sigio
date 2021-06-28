#include <stdexcept>
#include <cstring>

#include <syslog.h>
#include <time.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>

#include "sigio_.h"

namespace one
{
  void sigio::sigio_::sigaction_handler(int sig, siginfo_t *si, void *uc)
  {
    int fd = si->si_fd;
    auto& inst = get();
    if(!inst._io_map.count(fd))
    {
      auto ec = make_error_code(error::sig_handler_fd_unknown);
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      return;
    }
    
    // get user's call-back function
    inst._io_map[fd]._callback(si);
  }

  sigio::sigio_::sigio_(int sig):
    _signal(sig)
  {
    syslog(LOG_INFO, "sigio_ ctor with signal %d ", sig);
    int ret = 0;

    /* Establish handler for "I/O possible" signal */
    struct sigaction sa;
    sa.sa_sigaction = sigaction_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;

    if ((ret = sigaction(_signal, &sa, NULL)) != 0)
    {
      auto ec = std::make_error_code(static_cast<std::errc>(ret));
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }
  }

  sigio::sigio_& sigio::sigio_::get()
  {
    static sigio_ instance;
    return instance;
  }

  sigio::sigio_::~sigio_()
  {
    syslog(LOG_INFO, "sigio_::~sigio_()");

    // deactivate and erase all I/O file descriptors
    for(auto [fd, io]: _io_map)
    {
      auto ec = try_deactivate(fd);
      if(ec.value() != 0)
      {
        syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      }
    }
  }

  int sigio::sigio_::get_signal() const
  {
    return _signal;
  }

  std::error_code sigio::sigio_::set_signal(int sig)
  {
    _signal = sig;
    throw std::logic_error("set_signal funtion is not implemented yet");
  }

  void sigio::sigio_::activate(int fd, callback_t cb, seconds timeout_sec, nanoseconds timeout_nsec)
  {
    int ret = 0;
    int orig_flags = 0;

    if(_io_map.count(fd))
    {
      auto ec = make_error_code(error::activate_already_active);
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // Set owner process that is to receive "I/O possible" signal
    if ((ret = fcntl(fd, F_SETOWN, getpid())) != 0)
    {
      auto ec = std::make_error_code(static_cast<std::errc>(ret));
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // Enable "I/O possible" signaling and make I/O nonblocking for fd file descriptor
    orig_flags = fcntl(fd, F_GETFL);
    if ((ret = fcntl(fd, F_SETFL, orig_flags | O_ASYNC | O_NONBLOCK)) != 0)
    {
      auto ec = std::make_error_code(static_cast<std::errc>(ret));
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // setup a signal which will be fired in case I/O completion or other condiditions
    if ((ret = fcntl(fd, F_SETSIG, _signal)) != 0)
    {
      auto ec = std::make_error_code(static_cast<std::errc>(ret));
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // the file description fd is successfully configured for signal-driven I/O
    // add it to the map of descriptors
    _io_map[fd] = io_{orig_flags, cb, timeout_sec, timeout_nsec};
  }

  void sigio::sigio_::deactivate(int fd)
  {
    int ret = 0;

    if(!_io_map.count(fd))
    {
      auto ec = make_error_code(error::activate_already_active);
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // restore saved flags for file descriptor
    if ((ret = fcntl(fd, F_SETFL, _io_map[fd]._flags )) != 0)
    {
      auto ec = std::make_error_code(static_cast<std::errc>(ret));
      syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    // remove file descriptor from the I/O map
    _io_map.erase(fd);
  }

  bool sigio::sigio_::is_activated(int fd) const noexcept 
  {
    return _io_map.count(fd) ? true : false;
  }

  std::error_code sigio::sigio_::try_activate(int fd, callback_t cb, seconds timeout_sec, 
      nanoseconds timeout_nsec) noexcept
  {
    try
    {
      activate(fd, cb, timeout_sec, timeout_nsec);
    }
    catch (const std::system_error& e)
    {
      syslog(LOG_ERR, "error: %s", e.what());
      return e.code();
    }
    catch (...)
    {
      return make_error_code(sigio::error::unknown_error);
    }

    return make_error_code(error::success);
  }

  std::error_code sigio::sigio_::try_deactivate(int fd) noexcept
  {
    try
    {
      deactivate(fd);
    }
    catch (const std::system_error& e)
    {
      syslog(LOG_ERR, "error: %s", e.what());
      return e.code();
    }
    catch (...)
    {
      return make_error_code(sigio::error::unknown_error);
    }

    return make_error_code(error::success);
  }

} // namespace one
