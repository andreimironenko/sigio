/* STL C++ headers */
#include <stdexcept>

/* Local headers */
#include "sigio.h"
#include "sigio_.h"

namespace one 
{
  sigio::sigio():
    _sigio(&sigio_::get())
  {}

  sigio::~sigio() {}


  int sigio::get_signal() const
  {
    return _sigio->get_signal();
  }
  
  std::error_code sigio::set_signal(int signal)
  {
    return _sigio->set_signal(signal);
  }

  void sigio::activate(int fd, callback_t cb, seconds timeout_sec, nanoseconds timeout_nsec)
  {
    _sigio->activate(fd, cb, timeout_sec, timeout_nsec);
  }

  void sigio::deactivate(int fd)
  {
    _sigio->deactivate(fd);
  }

  bool sigio::is_activated(int fd) const noexcept
  {
    return _sigio->is_activated(fd);
  }

  std::error_code sigio::try_activate(int fd, callback_t cb,
      seconds timeout_sec, nanoseconds timeout_nsec) noexcept
  {
    return _sigio->try_activate(fd, cb, timeout_sec, timeout_nsec);
  }

  std::error_code sigio::try_deactivate(int fd) noexcept
  {
    return _sigio->try_deactivate(fd);
  }

} //namespace one 
