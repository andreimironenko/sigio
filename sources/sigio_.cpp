#include <stdexcept>
#include <cstring>

#include <syslog.h>
#include <time.h>

#include "timer_.h"

namespace posixcpp
{

  void timer::timer_::signal_handler(int sig, siginfo_t *si, void *uc)
  {
    // extracting timer_ object pointer from si_value.svial_ptr,
    auto tm = static_cast<timer_*>(si->si_value.sival_ptr);
    if (tm && sig == tm->_signal)
    {
      syslog(LOG_INFO, "timer_::signal_handler period(%lds, %ldns)", tm->_period_sec.count(), tm->_period_nsec.count());

      // calling user given callback function and passing data pointer
      tm->_callback(tm->_data);
    }
    else
    {
      if(!tm)
      {
        syslog(LOG_ERR, "timer_::signal_handler si->si_value.sival_ptr is nullptr, skipp handling");
      }
      else
      {
        syslog(LOG_ERR, "timer_::signal_handler si->si_value.sival_ptr is nullptr, skipp handling");
      }
    }
  }

  timer::timer_::timer_(std::chrono::seconds period_sec, std::chrono::nanoseconds period_nsec,
      callback_t callback, void* data,
      bool is_single_shot, int sig
      ):
    _period_sec(period_sec),
    _period_nsec(period_nsec),
    _callback(callback),
    _data(data),
    _is_single_shot(is_single_shot),
    _signal(sig),
    _timer(nullptr)
  {
    syslog(LOG_INFO, "timer_ ctor %ld sec, %ld nsec", period_sec.count(), period_nsec.count());

    _sa.sa_flags = SA_SIGINFO; /* Notify via signal */
    _sa.sa_sigaction = signal_handler;
    sigemptyset(&_sa.sa_mask); /* reset _sa structure data to all zeros */

    /* setup user defined signal and signal handler, third parameter as nullptr indicates, that we are
     * not interested in getting back early configured options
     */
    if (sigaction(_signal, &_sa, nullptr) != 0)    
    {
      auto ec = make_error_code(error::signal_handler_registration);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    /* Create and start one timer for each command-line argument */
    _sev.sigev_notify = SIGEV_SIGNAL;     /* Notify via signal */
    _sev.sigev_signo = sig;               /* Notify using this signal*/
    _sev.sigev_value.sival_ptr = this;    /* pointer to timer_ object will be passed to the signal handler*/

    /* creating a POSIX timer. IMPORTANT! here we pass timer_t which is stored in this class instance variable */
    if (timer_create(CLOCK_REALTIME, &_sev, &_timer) != 0)
    {
      auto ec = make_error_code(error::posix_timer_creation);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }
    syslog(LOG_INFO, "timer with period_nsec = %ld has created", period_nsec.count());
  }

  timer::timer_::~timer_()
  {
    syslog(LOG_INFO, "timer_::~timer_()");
    auto ec = try_stop();
    if (ec.value())
    { 
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
    }
    // POSIX will delte the timer
    timer_delete(_timer);
  }

  void timer::timer_::start()
  {
    struct itimerspec ts;

    syslog(LOG_INFO, "starting timer with period_sec = %ld, period_nsec = %ld", _period_sec.count(), _period_nsec.count());

    //get current time from timer
    if (timer_gettime(_timer, &ts) != 0)
    {
      auto ec = make_error_code(error::posix_timer_gettime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    if (ts.it_value.tv_sec != 0 || ts.it_value.tv_nsec != 0)
    {
      // timer is already started, doing nothing
      auto ec = make_error_code(error::start_already_started);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    _ts.it_value.tv_sec = _period_sec.count();
    _ts.it_value.tv_nsec = _period_nsec.count();

    if (!_is_single_shot) {
      _ts.it_interval.tv_sec = _ts.it_value.tv_sec;
      _ts.it_interval.tv_nsec = _ts.it_value.tv_nsec;
    }

    // oethrwise set to the defined value
    if (timer_settime(_timer, 0, &_ts, NULL) != 0)
    {
      auto ec = make_error_code(error::posix_timer_settime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    syslog(LOG_INFO, "timer started with preiod_sec = %ld, period_nsec = %ld", _period_sec.count(), _period_nsec.count());
  }

  void timer::timer_::reset()
  {
    stop();
    start();
  }

  void timer::timer_::suspend()
  {
    struct itimerspec ts;

    syslog(LOG_INFO, "trying to suspend timer 0x%lx", (unsigned long)(_timer));

    //get current time from timer
    if (timer_gettime(_timer, &ts) != 0)
    {
      auto ec = make_error_code(error::posix_timer_gettime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    if (ts.it_value.tv_sec == 0 && ts.it_value.tv_nsec == 0)
    {
      // timer is not started, doing nothing
      auto ec = make_error_code(error::suspend_while_not_running);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    try 
    {
      // copy current value
      std::memcpy(&_ts, &ts, sizeof(ts));
    }
    catch (const std::exception& e) 
    {
      auto ec = make_error_code(std::errc::not_enough_memory);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    ts.it_value.tv_sec = 0;
    ts.it_value.tv_nsec = 0;

    if (!_is_single_shot) {
      ts.it_interval.tv_sec = 0;
      ts.it_interval.tv_nsec = 0;
    }

    //disarm timer
    if (timer_settime(_timer, 0, &ts, NULL) != 0)
    {
      auto ec = make_error_code(error::posix_timer_settime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    syslog(LOG_INFO, "timer 0x%lx is suspended", (unsigned long)(_timer));
  }

  void timer::timer_::resume()
  {
    struct itimerspec ts;

    syslog(LOG_INFO, "trying to resume timer 0x%lx", (unsigned long)(_timer));

    if (timer_gettime(_timer, &ts) != 0)
    {
      auto ec = make_error_code(error::posix_timer_gettime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    syslog(LOG_INFO, "the timer 0x%lx ts: %ld, %ld, %ld, %ld", (unsigned long)(_timer),
        ts.it_value.tv_sec, ts.it_value.tv_nsec,
        ts.it_interval.tv_sec, ts.it_interval.tv_nsec
        );

    if (ts.it_value.tv_sec != 0 || ts.it_value.tv_nsec != 0)
    {
      auto ec = make_error_code(error::resume_already_running);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    if (timer_settime(_timer, 0, &_ts, NULL) != 0)
    {
      auto ec = make_error_code(error::posix_timer_settime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    syslog(LOG_INFO, "timer 0x%lx is resumed", (unsigned long)(_timer));
  }

  void timer::timer_::stop()
  {
    if (_ts.it_value.tv_sec == 0 && _ts.it_value.tv_nsec == 0 &&  
        _ts.it_interval.tv_sec == 0 && _ts.it_interval.tv_nsec == 0)
    {
      auto ec = make_error_code(error::stop_while_not_running);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);

    }

    syslog(LOG_INFO, "timer::timer_ trying to stop timer 0x%lx", (unsigned long)(_timer));
    _ts.it_value.tv_sec = 0;
    _ts.it_value.tv_nsec = 0;

    if (!_is_single_shot) {
      _ts.it_interval.tv_sec = 0;
      _ts.it_interval.tv_nsec = 0;
    }

    if (timer_settime(_timer, 0, &_ts, NULL) != 0)
    {
      auto ec = make_error_code(error::posix_timer_settime);
      syslog(LOG_ERR, "error %d: %s,", ec.value(), ec.message().c_str());
      throw std::system_error(ec);
    }

    syslog(LOG_INFO, "timer::timer_ stopped timer 0x%lX", (unsigned long)(_timer));
  }

  std::error_code timer::timer_::try_start() noexcept
  {
    try 
    {
      start();
    }
    catch (const std::system_error& e) 
    {
      return e.code();
    }
    catch (...)
    {
      return make_error_code(timer::error::unknown_error);
    }
    return std::make_error_code(static_cast<std::errc>(0));
  }

  std::error_code timer::timer_::try_reset() noexcept
  {
    try
    {
      reset();
    }
    catch (const std::system_error& e) 
    {
      return e.code();
    }
    catch (...)
    {
      return make_error_code(timer::error::unknown_error);
    }
    return std::make_error_code(static_cast<std::errc>(0));
  }

  std::error_code timer::timer_::try_suspend() noexcept
  {
    try
    {
      suspend();
    }
    catch (const std::system_error& e) 
    {
      return e.code();
    }
    catch (...)
    {
      return make_error_code(timer::error::unknown_error);
    }
    return std::make_error_code(static_cast<std::errc>(0));
  }

  std::error_code timer::timer_::try_resume() noexcept
  {
    try 
    {
      resume();
    }
    catch (const std::system_error& e) 
    {
      return e.code();
    }
    catch (...)
    {
      return make_error_code(timer::error::unknown_error);
    }
    return std::make_error_code(static_cast<std::errc>(0));
  }

  std::error_code timer::timer_::try_stop() noexcept
  {
    try 
    {
      stop();
    }
    catch (const std::system_error& e) 
    {
      return e.code();
    }
    catch (...)
    {
      return make_error_code(timer::error::unknown_error);
    }
    return std::make_error_code(static_cast<std::errc>(0));
  }

} // namespace posixcpp
