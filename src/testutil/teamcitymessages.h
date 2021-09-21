/* Copyright 2011 JetBrains s.r.o.
* Copyright 2015-2018 Alex Turbov <i.zaufi@gmail.com>
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

#pragma once

#include <iosfwd>
#include <string>

namespace jetbrains::teamcity
{
/// \name Helper functions
//@{
std::string getFlowIdFromEnvironment();
bool underTeamcity();
//@}

class TeamcityMessages
{
  std::ostream* m_out;

public:
  static const bool StdErr = true;
  static const bool StdOut = false;

  TeamcityMessages();

  void setOutput(std::ostream&);

  void suiteStarted(const std::string& name, const std::string& flowid = {});
  void suiteFinished(const std::string& name, const std::string& flowid = {});

  void testStarted(const std::string& name, const std::string& flowid = {}, bool captureStandardOutput = false);
  void testFinished(const std::string& name, const std::string& flowid = {});
  void testFinished(const std::string& name, unsigned long durationMs, const std::string& flowid = {});

  void testFailed(const std::string& name,
                  const std::string& message,
                  const std::string& details,
                  const std::string& flowid = {});
  void testIgnored(const std::string& name, const std::string& message, const std::string& flowid = {});

  void
    testOutput(const std::string& name, const std::string& output, const std::string& flowid, bool isStdErr = StdOut);
};

} // namespace jetbrains::teamcity
