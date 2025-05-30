/* Exception observers for outcome type
(C) 2017-2025 Niall Douglas <http://www.nedproductions.biz/> (3 commits)
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

#ifndef BOOST_OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_HPP
#define BOOST_OUTCOME_BASIC_OUTCOME_EXCEPTION_OBSERVERS_HPP

#include "basic_result_storage.hpp"

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  template <class Base, class R, class S, class P, class NoValuePolicy> class basic_outcome_exception_observers : public Base
  {
  public:
    using exception_type = P;
    using Base::Base;

    constexpr inline exception_type &assume_exception() & noexcept;
    constexpr inline const exception_type &assume_exception() const & noexcept;
    constexpr inline exception_type &&assume_exception() && noexcept;
    constexpr inline const exception_type &&assume_exception() const && noexcept;

    constexpr inline exception_type &exception() &;
    constexpr inline const exception_type &exception() const &;
    constexpr inline exception_type &&exception() &&;
    constexpr inline const exception_type &&exception() const &&;
  };

  // Exception observers not present
  template <class Base, class R, class S, class NoValuePolicy> class basic_outcome_exception_observers<Base, R, S, void, NoValuePolicy> : public Base
  {
  public:
    using Base::Base;

    constexpr void assume_exception() & noexcept { NoValuePolicy::narrow_exception_check(*this); }
    constexpr void assume_exception() const & noexcept { NoValuePolicy::narrow_exception_check(*this); }
    constexpr void assume_exception() && noexcept { NoValuePolicy::narrow_exception_check(std::move(*this)); }
    constexpr void assume_exception() const && noexcept { NoValuePolicy::narrow_exception_check(std::move(*this)); }

    constexpr void exception() & { NoValuePolicy::wide_exception_check(*this); }
    constexpr void exception() const & { NoValuePolicy::wide_exception_check(*this); }
    constexpr void exception() && { NoValuePolicy::wide_exception_check(std::move(*this)); }
    constexpr void exception() const && { NoValuePolicy::wide_exception_check(std::move(*this)); }
  };

}  // namespace detail

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
