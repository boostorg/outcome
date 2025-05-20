/* iostream specialisations for result and outcome
(C) 2017-2025 Niall Douglas <http://www.nedproductions.biz/> (21 commits)
File Created: July 2017


Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License in the accompanying file
Licence.txt or at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Distributed under the Boost Software License, Version 1.0.
    (See accompanying file Licence.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef OUTCOME_IOSTREAM_SUPPORT_HPP
#define OUTCOME_IOSTREAM_SUPPORT_HPP

#include "iostream_support_result.hpp"

#include "outcome.hpp"

OUTCOME_V2_NAMESPACE_BEGIN

/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P, class N)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<R>()),
                  OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<S>()),
                  OUTCOME_TEXPR(detail::lvalueref<std::istream>() >> detail::lvalueref<P>()))
inline std::istream &operator>>(std::istream &s, outcome<R, S, P, N> &v)
{
  s >> v._iostreams_state();
  if(v.has_error())
  {
    s >> v.assume_error();
  }
  if(v.has_exception())
  {
    s >> v.assume_exception();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
OUTCOME_TEMPLATE(class R, class S, class P, class N)
OUTCOME_TREQUIRES(OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<R>()),
                  OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<S>()),
                  OUTCOME_TEXPR(detail::lvalueref<std::ostream>() << detail::lvalueref<P>()))
inline std::ostream &operator<<(std::ostream &s, const outcome<R, S, P, N> &v)
{
  s << v._iostreams_state();
  if(v.has_error())
  {
    s << v.assume_error();
  }
  if(v.has_exception())
  {
    s << v.assume_exception();
  }
  return s;
}
/*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
template <class R, class S, class P, class N> inline std::string print(const outcome<R, S, P, N> &v)
{
  std::stringstream s;
  int total = static_cast<int>(v.has_value()) + static_cast<int>(v.has_error()) + static_cast<int>(v.has_exception());
  if(total > 1)
  {
    s << "{ ";
  }
  s << print(static_cast<const basic_result<R, S, N> &>(static_cast<const detail::basic_result_final<R, S, N> &>(v)));  // NOLINT
  if(total > 1)
  {
    s << ", ";
  }
  if(v.has_exception())
  {
#ifdef __cpp_exceptions
    try
    {
      rethrow_exception(v.exception());
    }
    catch(const std::system_error &e)
    {
      s << "std::system_error code " << e.code() << ": " << e.what();
    }
    catch(const std::exception &e)
    {
      s << "std::exception: " << e.what();
    }
    catch(...)
#endif
    {
      s << "unknown exception";
    }
  }
  if(total > 1)
  {
    s << " }";
  }
  return s.str();
}
OUTCOME_V2_NAMESPACE_END

#endif
