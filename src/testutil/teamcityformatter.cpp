/* Copyright 2011 JetBrains s.r.o.
* Copyright 2015-2020 Alex Turbov <i.zaufi@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* $Revision: 88625 $
*/

#include "teamcitymessages.h"

#include <boost/test/execution_monitor.hpp>
#include <boost/test/results_collector.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test_log_formatter.hpp>
#include <boost/test/utils/basic_cstring/io.hpp>
#include <cstddef>
#include <ostream>
#include <string>
#include <utility>

namespace jetbrains::teamcity
{
namespace
{
const char* ASSERT_CTX = "Assertion has occurred in a following context:";
const char* FAILURE_CTX = "Failure has occurred in a following context:";
const boost::unit_test::test_unit_type UNIT_TEST_CASE = boost::unit_test::TUT_CASE;

// Formatter implementation
std::string toString(const boost::unit_test::const_string& bstr)
{
  std::stringstream ss(std::ios_base::out);
  ss << bstr;
  return ss.str();
}

std::string toString(const boost::execution_exception::error_code code)
{
  switch(code)
  {
  case boost::execution_exception::no_error:
    return {};
  case boost::execution_exception::user_error:
    return "user error";
  case boost::execution_exception::cpp_exception_error:
    return "C++ exception error";
  case boost::execution_exception::system_error:
    return "system error";
  case boost::execution_exception::user_fatal_error:
    return "user fatal error";
  case boost::execution_exception::system_fatal_error:
    return "system fatal error";
  case boost::execution_exception::timeout_error:
    return "timeout error";
  default:
    break;
  }
  return "unknown error";
}

std::string toString(const boost::execution_exception& excpt)
{
  std::stringstream ss(std::ios_base::out);
  ss << excpt.where().m_file_name;

  size_t line = excpt.where().m_line_num;
  if(line)
    ss << ':' << line;

  const boost::unit_test::const_string& fn = excpt.where().m_function;
  if(!fn.empty())
    ss << '[' << fn << ']';

  ss << ": " << excpt.what();

  const std::string& code = toString(excpt.code());
  if(!code.empty())
    ss << " (" << code << ')';

  return ss.str();
}
} // anonymous namespace

/// Custom formatter for TeamCity messages
class TeamcityBoostLogFormatter : public boost::unit_test::unit_test_log_formatter
{
  TeamcityMessages m_messages{};
  std::string m_currentDetails{};
  std::string m_flowId{};
  std::string m_currentContextDetails{};

public:
  using boost::unit_test::unit_test_log_formatter::log_entry_value;
  using boost::unit_test::unit_test_log_formatter::test_unit_skipped;

  explicit TeamcityBoostLogFormatter(std::string flowId);
  TeamcityBoostLogFormatter();

  void log_start(std::ostream&, boost::unit_test::counter_t) override;
  void log_finish(std::ostream&) override;
  void log_build_info(std::ostream&
#if BOOST_VERSION >= 107000
                      ,
                      bool
#endif // BOOST_VERSION >= 107000
                      ) override;
  void test_unit_start(std::ostream&, const boost::unit_test::test_unit&) override;
  void test_unit_finish(std::ostream&, const boost::unit_test::test_unit&, unsigned long) override;

  void log_entry_start(std::ostream&, const boost::unit_test::log_entry_data&, log_entry_types) override;
  void log_entry_value(std::ostream&, boost::unit_test::const_string) override;
  void log_entry_finish(std::ostream&) override;

  void log_exception_start(std::ostream&,
                           const boost::unit_test::log_checkpoint_data&,
                           const boost::execution_exception&) override;
  void test_unit_skipped(std::ostream&, const boost::unit_test::test_unit&, boost::unit_test::const_string) override;

  void log_exception_finish(std::ostream&) override;
  void entry_context_start(std::ostream&, boost::unit_test::log_level) override;
  void log_entry_context(std::ostream&, boost::unit_test::log_level, boost::unit_test::const_string) override;
  void entry_context_finish(std::ostream&, boost::unit_test::log_level) override;
  [[nodiscard]] std::string get_default_stream_description() const override
  {
    return "TeamCity (via service messages)";
  }
};

// Fake fixture to register formatter
struct TeamcityFormatterRegistrar
{
  TeamcityFormatterRegistrar()
  {
    if(underTeamcity())
    {
      boost::unit_test::unit_test_log.set_formatter(new TeamcityBoostLogFormatter());
      boost::unit_test::unit_test_log.set_threshold_level(boost::unit_test::log_test_units);
    }
  }
};

BOOST_TEST_GLOBAL_CONFIGURATION(TeamcityFormatterRegistrar);

TeamcityBoostLogFormatter::TeamcityBoostLogFormatter()
    : m_flowId(getFlowIdFromEnvironment())
{
}

TeamcityBoostLogFormatter::TeamcityBoostLogFormatter(std::string id)
    : m_flowId(std::move(id))
{
}

void TeamcityBoostLogFormatter::log_start(std::ostream& out, boost::unit_test::counter_t /*test_cases_amount*/
)
{
  m_messages.setOutput(out);
}

void TeamcityBoostLogFormatter::log_finish(std::ostream& /*out*/)
{
}

void TeamcityBoostLogFormatter::log_build_info(std::ostream& /*out*/
#if BOOST_VERSION >= 107000
                                               ,
                                               const bool /*log_build_info*/
#endif
)
{
}

void TeamcityBoostLogFormatter::test_unit_start(std::ostream& /*out*/
                                                ,
                                                const boost::unit_test::test_unit& tu)
{
  if(tu.p_type == UNIT_TEST_CASE)
    m_messages.testStarted(tu.p_name, m_flowId);
  else
    m_messages.suiteStarted(tu.p_name, m_flowId);

  m_currentDetails.clear();
}

void TeamcityBoostLogFormatter::test_unit_finish(std::ostream& /*out*/
                                                 ,
                                                 const boost::unit_test::test_unit& tu,
                                                 const unsigned long elapsed)
{
  const boost::unit_test::test_results& tr = boost::unit_test::results_collector.results(tu.p_id);
  if(tu.p_type == UNIT_TEST_CASE)
  {
    if(!tr.passed())
    {
      if(tr.p_skipped)
        m_messages.testIgnored(tu.p_name, "ignored", m_flowId);
      else if(tr.p_aborted)
        m_messages.testFailed(tu.p_name, "aborted", m_currentDetails, m_flowId);
      else
        m_messages.testFailed(tu.p_name, "failed", m_currentDetails, m_flowId);

      if(!m_currentContextDetails.empty())
      {
        m_messages.testOutput(tu.p_name, m_currentContextDetails, m_flowId, TeamcityMessages::StdErr);
        m_currentContextDetails.clear();
      }
    }

    m_messages.testFinished(tu.p_name, elapsed / 1000, m_flowId);
  }
  else
  {
    m_messages.suiteFinished(tu.p_name, m_flowId);
  }
}

void TeamcityBoostLogFormatter::log_entry_start(std::ostream& /*out*/
                                                ,
                                                const boost::unit_test::log_entry_data& entry_data,
                                                log_entry_types /*let*/
)
{
  std::stringstream ss(std::ios_base::out);
  ss << entry_data.m_file_name << ':' << entry_data.m_line_num << ": ";
  m_currentDetails += ss.str();
}

void TeamcityBoostLogFormatter::log_entry_value(std::ostream& /*out*/
                                                ,
                                                boost::unit_test::const_string value)
{
  m_currentDetails += toString(value);
}

void TeamcityBoostLogFormatter::log_entry_finish(std::ostream& /*out*/)
{
  m_currentDetails += '\n';
}

void TeamcityBoostLogFormatter::log_exception_start(std::ostream& /*out*/
                                                    ,
                                                    const boost::unit_test::log_checkpoint_data& cp,
                                                    const boost::execution_exception& excpt)
{
  const std::string what = toString(excpt);
  m_currentDetails += what + '\n';
  if(!cp.m_file_name.empty())
  {
    std::stringstream ss(std::ios_base::out);
    ss << "Last checkpoint at " << cp.m_file_name << ':' << cp.m_line_num;
    if(!cp.m_message.empty())
      ss << ": " << cp.m_message;
    m_currentDetails += ss.str();
  }
}

void TeamcityBoostLogFormatter::log_exception_finish(std::ostream& /*out*/)
{
}

void TeamcityBoostLogFormatter::test_unit_skipped(std::ostream& /*out*/
                                                  ,
                                                  const boost::unit_test::test_unit& tu,
                                                  boost::unit_test::const_string reason)
{
  m_messages.testIgnored(tu.p_name, toString(reason), m_flowId);
}

void TeamcityBoostLogFormatter::entry_context_start(std::ostream& /*out*/
                                                    ,
                                                    boost::unit_test::log_level l)
{
  const char* initial_msg = (l == boost::unit_test::log_successful_tests ? ASSERT_CTX : FAILURE_CTX);
  m_currentContextDetails = initial_msg;
}

void TeamcityBoostLogFormatter::log_entry_context(std::ostream& /*out*/
                                                  ,
                                                  boost::unit_test::log_level,
                                                  boost::unit_test::const_string ctx)
{
  m_currentContextDetails += '\n' + toString(ctx);
}

void TeamcityBoostLogFormatter::entry_context_finish(std::ostream& /*out*/, boost::unit_test::log_level)
{
}

} // namespace jetbrains::teamcity
