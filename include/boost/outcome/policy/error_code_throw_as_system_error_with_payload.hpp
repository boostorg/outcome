/* Policies for result and outcome
(C) 2017 Niall Douglas <http://www.nedproductions.biz/> (59 commits)
File Created: Oct 2017


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef BOOST_OUTCOME_POLICY_ERROR_CODE_THROW_AS_SYSTEM_ERROR_WITH_PAYLOAD_HPP
#define BOOST_OUTCOME_POLICY_ERROR_CODE_THROW_AS_SYSTEM_ERROR_WITH_PAYLOAD_HPP

#include "../bad_access.hpp"
#include "detail/common.hpp"

#include <system_error>

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

#ifdef STANDARDESE_IS_IN_THE_HOUSE
template <class R, class S, class P, class N> class outcome;
#endif

namespace policy
{
  /*! Policy interpreting S as a type implementing the `std::error_code` contract, P
  as a payload type, and any wide attempt to access the successful state calls an
  ADL discovered free function `throw_as_system_error_with_payload()`.

  Can be used in `outcome` only.
  */
  template <class R, class S, class P> struct error_code_throw_as_system_error_with_payload : detail::base
  {
    /*! Performs a wide check of state, used in the value() functions.
    \effects If outcome does not have a value,
    if has an error it throws a `std::system_error(error())`, else it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_value_check(Impl *self)
    {
      if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_value) == 0)
      {
        if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_payload) != 0)
        {
          auto *_self = static_cast<const outcome<R, S, P, error_code_throw_as_system_error_with_payload> *>(self);
          throw_as_system_error_with_payload(_self);
        }
        if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_error) != 0)
        {
          BOOST_OUTCOME_THROW_EXCEPTION(std::system_error(self->_error));
        }
        BOOST_OUTCOME_THROW_EXCEPTION(bad_outcome_access("no value"));
      }
    }
    /*! Performs a wide check of state, used in the error() functions
    \effects If outcome does not have an error, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_error_check(Impl *self)
    {
      if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_error) == 0)
      {
        BOOST_OUTCOME_THROW_EXCEPTION(bad_outcome_access("no error"));
      }
    }
    /*! Performs a wide check of state, used in the payload() functions
    \effects If outcome does not have a payload, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_payload_check(Impl *self)
    {
      if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_payload) == 0)
      {
        BOOST_OUTCOME_THROW_EXCEPTION(bad_outcome_access("no payload"));
      }
    }
    /*! Performs a wide check of state, used in the exception() functions
    \effects If outcome does not have an exception, it throws `bad_outcome_access`.
    */
    template <class Impl> static constexpr void wide_exception_check(Impl *self)
    {
      if((self->_state._status & BOOST_OUTCOME_V2_NAMESPACE::detail::status_have_exception) == 0)
      {
        BOOST_OUTCOME_THROW_EXCEPTION(bad_outcome_access("no exception"));
      }
    }
  };
}  // namespace policy

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
