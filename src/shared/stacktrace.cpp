#include "stacktrace.h"

#include <boost/log/trivial.hpp>
#include <boost/stacktrace.hpp>

namespace stacktrace
{
void logStacktrace()
{
  BOOST_LOG_TRIVIAL(fatal) << "Stacktrace:";
  for(const auto& frame : boost::stacktrace::stacktrace{})
  {
    BOOST_LOG_TRIVIAL(fatal) << "0x" << frame.address() << " " << frame.source_file() << ":" << frame.source_line()
                             << " " << frame.name();
  }
}
} // namespace stacktrace
