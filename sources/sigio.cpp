/* STL C++ headers */
#include <stdexcept>

/* Local headers */
#include "timer.h"
#include "timer_.h"

namespace posixcpp
{
  timer::timer(std::chrono::seconds period_sec, std::chrono::nanoseconds period_nsec,
      callback_t callback, void* data, bool is_single_shot, int sig) :
    _timer(new timer_(period_sec, period_nsec, callback, data, is_single_shot, sig))
  {}

  timer::~timer()
  {
    syslog(LOG_INFO, "timer::~timer()");
  }

  void timer::start()
  {
    _timer->start();
  }

  void timer::reset()
  {
    _timer->reset();
  }

  void timer::suspend()
  {
    _timer->suspend();
  }

  void timer::resume()
  {
    _timer->resume();
  }

  void timer::stop()
  {
    _timer->stop();
  }

  std::error_code timer::try_start() noexcept
  {
    return _timer->try_start();
  }

  std::error_code timer::try_reset() noexcept
  {
    return _timer->try_reset();
  }

  std::error_code timer::try_suspend() noexcept
  {
    return _timer->try_suspend();
  }

  std::error_code timer::try_resume() noexcept
  {
    return _timer->try_resume();
  }

  std::error_code timer::try_stop() noexcept
  {
    return _timer->try_stop();
  }

} //namespace posixcpp
