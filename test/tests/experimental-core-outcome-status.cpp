/* Unit testing for outcomes
(C) 2013-2025 Niall Douglas <http://www.nedproductions.biz/> (4 commits)


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

#include <boost/outcome/experimental/status_outcome.hpp>

#define BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND std
template <class T, class S = BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE::system_code, class P = BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::exception_ptr> using outcome = BOOST_OUTCOME_V2_NAMESPACE::experimental::status_outcome<T, S, P>;
using BOOST_OUTCOME_V2_NAMESPACE::in_place_type;

#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(disable : 4702)  // unreachable code
#endif

BOOST_OUTCOME_AUTO_TEST_CASE(works_status_code_outcome, "Tests that the outcome with status_code works as intended")
{
  using namespace BOOST_OUTCOME_SYSTEM_ERROR2_NAMESPACE;

  {  // errored int
    outcome<int> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), status_error<void>);
    BOOST_CHECK_NO_THROW(m.error());
    BOOST_CHECK_THROW(BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::rethrow_exception(m.failure()), generic_error);
  }
  {  // errored void
    outcome<void> m(generic_code{errc::bad_address});
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(([&m]() -> void { return m.value(); }()), generic_error);
    BOOST_CHECK_NO_THROW(m.error());
    BOOST_CHECK_THROW(BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::rethrow_exception(m.failure()), generic_error);
  }
  {  // valued int
    outcome<int> m(5);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == 5);
    m.value() = 6;
    BOOST_CHECK(m.value() == 6);
    BOOST_CHECK(!m.failure());
  }
  {  // moves do not clear state
    outcome<std::string> m("niall");
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == "niall");
    m.value() = "NIALL";
    BOOST_CHECK(m.value() == "NIALL");
    auto temp(std::move(m).value());
    BOOST_CHECK(temp == "NIALL");
    BOOST_CHECK(m.value().empty());  // NOLINT
  }
  {  // valued void
    outcome<void> m(in_place_type<void>);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_NO_THROW(m.value());  // works, but type returned is unusable
    BOOST_CHECK(!m.failure());
  }
  {  // errored
    error ec(errc::no_link);
    outcome<int> m(ec.clone());
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), generic_error);
    BOOST_CHECK(m.error() == ec);
#ifndef BOOST_NO_EXCEPTIONS
    BOOST_CHECK(m.failure());
    try
    {
      BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::rethrow_exception(m.failure());
    }
    catch(const generic_error &ex)
    {
      BOOST_CHECK(ex.code() == ec);
      BOOST_CHECK(ex.code().value() == errc::no_link);
    }
#endif
  }
#if !defined(__APPLE__) || defined(__cpp_exceptions)
  {  // excepted
    BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::error_code ec(5, BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_category());
    auto e = BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::make_exception_ptr(BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_error(ec));  // NOLINT
    outcome<int> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
    BOOST_CHECK_THROW(m.value(), BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_error);
    BOOST_CHECK(m.exception() == e);
#ifndef BOOST_NO_EXCEPTIONS
    BOOST_CHECK(m.failure());
    try
    {
      BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::rethrow_exception(m.failure());
    }
    catch(const BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_error &ex)
    {
      BOOST_CHECK(ex.code() == ec);
      BOOST_CHECK(ex.code().value() == 5);
    }
#endif
  }
  {  // custom error type
    struct Foo
    {
    };
    auto e = BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::make_exception_ptr(Foo());
    outcome<int> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
    BOOST_CHECK_THROW(m.value(), Foo);
    BOOST_CHECK(m.exception() == e);
  }
  {  // outcome<void, void> should work
    BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::error_code ec(5, BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_category());
    auto e = BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::make_exception_ptr(BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::system_error(ec));
    outcome<void, void> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
  }
#endif


  {
    outcome<int> a(5);
    outcome<int> b(generic_code{errc::invalid_argument});
    std::cout << sizeof(a) << std::endl;  // 40 bytes
    a.assume_value();
    b.assume_error();
#ifndef BOOST_NO_EXCEPTIONS
    try
    {
      b.value();
      std::cerr << "fail" << std::endl;
      std::terminate();
    }
    catch(const generic_error & /*unused*/)
    {
    }
#endif
    static_assert(!std::is_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(a)>::value, "");

    // Test void compiles
    outcome<void> c(in_place_type<void>);
    // Test int, void compiles
    outcome<int, void> d(in_place_type<BOOST_OUTCOME_PREVENT_CONVERSION_WORKAROUND::exception_ptr>);
  }

  {
    // Can only be constructed via multiple args
    struct udt3
    {
      udt3() = delete;
      udt3(udt3 &&) = delete;
      udt3(const udt3 &) = delete;
      udt3 &operator=(udt3 &&) = delete;
      udt3 &operator=(const udt3 &) = delete;
      explicit udt3(int /*unused*/, const char * /*unused*/, std::nullptr_t /*unused*/) {}
      ~udt3() = default;
    };
    // Test a udt which can only be constructed in place compiles
    outcome<udt3> g(in_place_type<udt3>, 5, static_cast<const char *>("niall"), nullptr);
    // Does converting inplace construction also work?
    outcome<udt3> h(5, static_cast<const char *>("niall"), nullptr);
    BOOST_CHECK(h.has_value());
  }
}
