#include <stdexcept>
#include <cstring>

#include <syslog.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "sigio_.h"

namespace one
{
  void* sigio::sigio_::sig_thread(void *arg)
  {
      sigset_t* set = (sigset_t*)arg;
      int sig;
      siginfo_t si;
      auto& inst = get();

      while(true)
      {
          if((sig = sigwaitinfo(set, &si)) == -1)
          {
              auto ec = std::make_error_code(static_cast<std::errc>(sig));
              syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
              continue;
          }
          int fd = si.si_fd;
          if(!inst._io.count(fd))
          {
              auto ec = make_error_code(error::sig_handler_fd_unknown);
              syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
              continue;
          }

          // get user's call-back function
          inst._io[fd]._callback(&si);
      }
      return arg;
  }

  void sigio::sigio_::inotify_handler(siginfo_t* si)
  {
      size_t nbytes;
      auto& inst = get();
      // check if this is inotify device
      if(si->si_fd != inst._inotify_fd)
      {
          auto ec = make_error_code(error::sig_handler_fd_unknown);
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          return;
      }
      // check ho
      ioctl(si->si_fd, FIONREAD, &nbytes);

      for (auto& [name, io] : inst._inotify_io)
      {
      }
  }

  sigio::sigio_::sigio_(int sig):
      _signal(sig)
  {
      syslog(LOG_INFO, "sigio_ ctor with signal %d ", sig);
      int ret = 0;

      sigemptyset(&_sig_set);
      sigaddset(&_sig_set, _signal);

      if((ret = pthread_sigmask(SIG_BLOCK, &_sig_set, NULL)) !=0)
      {
          auto ec = std::make_error_code(static_cast<std::errc>(ret));
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          throw std::system_error(ec);
      }

      if((ret = pthread_create(&_sig_thread, NULL, &sig_thread, (void*)&_sig_set)) !=0)
      {
          auto ec = std::make_error_code(static_cast<std::errc>(ret));
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          throw std::system_error(ec);
      }

      // open inotify device
      if ((_inotify_fd = inotify_init()) == -1 )
      {
          auto ec = std::make_error_code(static_cast<std::errc>(errno));
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          throw std::system_error(ec);
      }

      // activate inotify device entry
      activate(_inotify_fd, inotify_handler);
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

    if(_io.count(fd))
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
    _io[fd] = io_{orig_flags, cb, timeout_sec, timeout_nsec};
  }

  void sigio::sigio_::activate(const std::string fn, uint32_t inotify_mask, inotify_callback_t cb,
                               seconds timeout_sec, nanoseconds timeout_nsec)
  {


  }

  void sigio::sigio_::deactivate(int fd)
  {
      int ret = 0;

      if(!is_activated(fd))
      {
          auto ec = make_error_code(error::deactivate_not_active);
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          throw std::system_error(ec);
      }

      // restore saved flags for file descriptor
      if ((ret = fcntl(fd, F_SETFL, _io[fd]._flags )) != 0)
      {
          auto ec = std::make_error_code(static_cast<std::errc>(ret));
          syslog(LOG_ERR, "error %d: %s", ec.value(), ec.message().c_str());
          throw std::system_error(ec);
      }

      // remove file descriptor from the I/O map
      _io.erase(fd);
  }

  void sigio::sigio_::deactivate(const std::string fn)
  {
      int fd = fn.size();
      deactivate(fd);
  }


  bool sigio::sigio_::is_activated(int fd) const noexcept 
  {
    return _io.count(fd) ? true : false;
  }

  bool sigio::sigio_::is_activated(const std::string fn) const noexcept
  {
      int fd = fn.size();
      return is_activated(fd);
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

  std::error_code sigio::sigio_::try_activate(const std::string fn, uint32_t inotify_mask, inotify_callback_t cb,
                                              seconds timeout_sec, nanoseconds timeout_nsec) noexcept
  {
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


  std::error_code sigio::sigio_::try_deactivate(const std::string fn) noexcept
  {
      int fd = fn.size();
      return try_deactivate(fd);
  }

} // namespace one
