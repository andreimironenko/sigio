/* STL C++ headers */
#include <stdexcept>

/* C++ Boost library headers */
#include <boost/python.hpp>
#include <boost/python/call.hpp>

/* Local headers */
#include "timer.h"

using namespace boost::python;

namespace posixpy
{
  struct timer: public posixcpp::timer
  {
    explicit timer(long period_sec, long period_nsec, PyObject* py_callback, PyObject* py_data,
        bool is_single_shot, int sig):
      posixcpp::timer(
          std::chrono::seconds(period_sec), std::chrono::nanoseconds(period_nsec),
          [py_callback](void* data){return boost::python::call<void>(py_callback, data);},
          (void*) py_data,
          is_single_shot, sig)
      {}

    timer(const timer&) = delete; 
    timer(timer&&) = delete;
    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;
    ~timer() = default;
  };

  BOOST_PYTHON_MODULE(posixpy_timer)
  {
    class_<posixpy::timer>("timer", init<long, long, bool, int>())
      .def("start", &timer::start)
      .def("reset", &timer::reset)
      .def("suspend", &timer::suspend)
      .def("resume", &timer::resume)
      .def("stop", &timer::stop)
      ;
  }
} //namespace posixcpp
