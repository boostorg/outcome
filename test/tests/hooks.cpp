/* Unit testing for outcomes
(C) 2013-2025 Niall Douglas <http://www.nedproductions.biz/> (14 commits)


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

#define _CRT_SECURE_NO_WARNINGS
#define BOOST_OUTCOME_ENABLE_LEGACY_SUPPORT_FOR 210  // legacy ADL hooks support

#include <boost/outcome.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>

#include <iostream>

namespace hook_test
{
  using BOOST_OUTCOME_V2_NAMESPACE::in_place_type;
  // Use static storage to convey extended error info from result construction to outcome conversion
  static char extended_error_info[256];

  // Use the error_code type as the ADL bridge for the hooks by creating a type here
  struct error_code : public boost::system::error_code
  {
    error_code() = default;
    error_code(boost::system::error_code ec)  // NOLINT
    : boost::system::error_code(ec)
    {
    }
  };
  // Localise result to using the local error_code so this namespace gets looked up for the hooks
  template <class R> using result = BOOST_OUTCOME_V2_NAMESPACE::result<R, error_code>;
  // Specialise the result construction hook for our localised result
  template <class U> constexpr inline void hook_result_construction(result<int> *res, U && /*unused*/) noexcept
  {
    // Write the value in the result into the static storage
    snprintf(extended_error_info, sizeof(extended_error_info), "%d", res->assume_value());  // NOLINT
  }
  template <class U> constexpr inline void hook_result_construction(result<std::string> *res, U && /*unused*/) noexcept
  {
    // Write the value in the result into the static storage
    snprintf(extended_error_info, sizeof(extended_error_info), "%s", res->assume_value().c_str());  // NOLINT
  }
}  // namespace hook_test

BOOST_OUTCOME_AUTO_TEST_CASE(works_result_hooks, "Tests that you can hook result's construction")
{
  using namespace hook_test;
  result<int> a(5);
  BOOST_CHECK(!strcmp(extended_error_info, "5"));  // NOLINT
  result<std::string> b("niall");
  BOOST_CHECK(!strcmp(extended_error_info, "niall"));  // NOLINT
}

//! [extended_error_coding2]
namespace hook_test
{
  // Localise outcome to using the local error_code so this namespace gets looked up for the hooks
  template <class R> using outcome = BOOST_OUTCOME_V2_NAMESPACE::outcome<R, error_code, std::string>;

  // Specialise the outcome copy and move conversion hook for our localised result
  template <class T, class U> constexpr inline void hook_outcome_copy_construction(outcome<T> *res, const result<U> & /*unused*/) noexcept
  {
    // when copy constructing from a result<T>, place extended_error_coding::extended_error_info into the payload
    std::cout << "hook_outcome_copy_construction fires" << std::endl;
    BOOST_OUTCOME_V2_NAMESPACE::hooks::override_outcome_exception(res, extended_error_info);
  }
  template <class T, class U> constexpr inline void hook_outcome_move_construction(outcome<T> *res, result<U> && /*unused*/) noexcept
  {
    // when move constructing from a result<T>, place extended_error_coding::extended_error_info into the payload
    std::cout << "hook_outcome_move_construction fires" << std::endl;
    BOOST_OUTCOME_V2_NAMESPACE::hooks::override_outcome_exception(res, extended_error_info);
  }
}  // namespace hook_test

BOOST_OUTCOME_AUTO_TEST_CASE(works_outcome_hooks, "Tests that you can hook outcome's conversion from a result")
{
  using namespace hook_test;
  outcome<int> a(result<int>(5));
  BOOST_REQUIRE(a.has_exception());  // NOLINT
  BOOST_CHECK(a.exception() == "5");
  outcome<std::string> b(result<std::string>("niall"));
  BOOST_CHECK(b.exception() == "niall");

  // Make sure hook does not fire for any other kind of outcome copy or move, only when converting from our custom result only
  outcome<int> c(5);
  outcome<long> d(c);  // can't be the same type as source, else copy elision takes place and no ADL hook calling
  BOOST_CHECK(!d.has_exception());
  outcome<int> e(BOOST_OUTCOME_V2_NAMESPACE::result<int>(5));
  BOOST_CHECK(!e.has_exception());
}
