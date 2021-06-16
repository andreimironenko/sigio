/* STL C++ headers */
#include <stdexcept>

/* C++ Boost library headers */
#include <boost/python.hpp>

/* Local headers */
#include "timer.h"
#include "timer_.h"

using namespace boost::python;

namespace posixcpp
{

  BOOST_PYTHON_MODULE(libposixcpp_timer)
  {
    class_<timer>("timer", init<long, long, void (*)(void*), void*, bool, int>())
      .def("start", &timer::start)
      .def("reset", &timer::reset)
      .def("suspend", &timer::suspend)
      .def("resume", &timer::resume)
      .def("stop", &timer::stop)
      ;
  }
} //namespace posixcpp
