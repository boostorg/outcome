/* Unit testing for outcomes
(C) 2013-2025 Niall Douglas <http://www.nedproductions.biz/> (18 commits)


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

#include <boost/outcome.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>

#include <iostream>

#ifdef _MSC_VER
#pragma warning(disable : 4702)  // unreachable code
#endif

BOOST_OUTCOME_AUTO_TEST_CASE(works_outcome, "Tests that the outcome works as intended")
{
  using namespace BOOST_OUTCOME_V2_NAMESPACE;

  static_assert(std::is_constructible<outcome<long>, int>::value, "Sanity check that monad can be constructed from a value_type");
  static_assert(!std::is_constructible<outcome<outcome<long>>, int>::value,
                "Sanity check that outer monad can be constructed from an inner monad's value_type");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9  // GCCs before 9 barf on this
  static_assert(!std::is_constructible<outcome<outcome<outcome<long>>>, int>::value,
                "Sanity check that outer monad can be constructed from an inner inner monad's value_type");
  static_assert(!std::is_constructible<outcome<outcome<outcome<outcome<long>>>>, int>::value,
                "Sanity check that outer monad can be constructed from an inner inner monad's value_type");
#endif

  static_assert(std::is_constructible<outcome<int>, outcome<long>>::value, "Sanity check that compatible monads can be constructed from one another");
  static_assert(std::is_constructible<outcome<outcome<int>>, outcome<long>>::value, "Sanity check that outer monad can be constructed from a compatible monad");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9  // GCCs before 9 barf on this
  static_assert(!std::is_constructible<outcome<outcome<outcome<int>>>, outcome<long>>::value,
                "Sanity check that outer monad can be constructed from a compatible monad up to two nestings deep");
  static_assert(!std::is_constructible<outcome<outcome<outcome<outcome<int>>>>, outcome<long>>::value,
                "Sanity check that outer monad can be constructed from a compatible monad three or more nestings deep");
#endif
  static_assert(!std::is_constructible<outcome<std::string>, outcome<int>>::value,
                "Sanity check that incompatible monads cannot be constructed from one another");

  static_assert(std::is_constructible<outcome<int>, outcome<void>>::value, "Sanity check that all monads can be constructed from a void monad");
  static_assert(std::is_constructible<outcome<outcome<int>>, outcome<void>>::value, "Sanity check that outer monad can be constructed from a compatible monad");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9 // GCCs before 9 barf on this
  static_assert(std::is_constructible<outcome<outcome<outcome<int>>>, outcome<void>>::value,
                "Sanity check that outer monad can be constructed from a compatible monad up to two nestings deep");
#endif
  static_assert(!std::is_constructible<outcome<void>, outcome<int>>::value, "Sanity check that incompatible monads cannot be constructed from one another");

  static_assert(std::is_void<result<void>::value_type>::value, "Sanity check that result<void> has a void value_type");
  static_assert(std::is_void<result<void, void>::error_type>::value, "Sanity check that result<void, void> has a void error_type");
  // static_assert(std::is_void<outcome<void, void, void>::exception_type>::value, "Sanity check that outcome<void, void, void> has a void exception_type");

  static_assert(std::is_same<outcome<int>::value_type, int>::value, "Sanity check that outcome<int> has a int value_type");
  static_assert(std::is_same<outcome<int>::error_type, boost::system::error_code>::value, "Sanity check that outcome<int> has a error_code error_type");
  static_assert(std::is_same<outcome<int>::exception_type, boost::exception_ptr>::value, "Sanity check that outcome<int> has a exception_ptr exception_type");


  {  // errored int
    outcome<int> m(boost::system::errc::bad_address);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK_NO_THROW(m.error());
    BOOST_CHECK_THROW(m.exception(), bad_outcome_access);
    BOOST_CHECK_THROW(boost::rethrow_exception(m.failure()), boost::system::system_error);
  }
  {  // errored void
    outcome<void> m(boost::system::errc::bad_address);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(([&m]() -> void { return m.value(); }()), boost::system::system_error);
    BOOST_CHECK_NO_THROW(m.error());
    BOOST_CHECK_THROW(m.exception(), bad_outcome_access);
    BOOST_CHECK_THROW(boost::rethrow_exception(m.failure()), boost::system::system_error);
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
    BOOST_CHECK_THROW(m.error(), bad_outcome_access);
    BOOST_CHECK_THROW(m.exception(), bad_outcome_access);
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
    BOOST_CHECK_THROW(m.error(), bad_outcome_access);
    BOOST_CHECK_THROW(m.exception(), bad_outcome_access);
    BOOST_CHECK(!m.failure());
  }
  {  // errored
    boost::system::error_code ec(5, boost::system::system_category());
    outcome<int> m(ec);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK(m.error() == ec);
    BOOST_CHECK_THROW(m.exception(), bad_outcome_access);
#ifndef BOOST_NO_EXCEPTIONS
    BOOST_CHECK(m.failure());
    try
    {
      boost::rethrow_exception(m.failure());
    }
    catch(const boost::system::system_error &ex)
    {
      BOOST_CHECK(ex.code() == ec);
      BOOST_CHECK(ex.code().value() == 5);
    }
#endif
  }
#if !defined(__APPLE__) || defined(__cpp_exceptions)
  {  // excepted
    boost::system::error_code ec(5, boost::system::system_category());
    auto e = boost::copy_exception(boost::system::system_error(ec));  // NOLINT
    outcome<int> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK_THROW(m.error(), bad_outcome_access);
    BOOST_CHECK(m.exception() == e);
#ifndef BOOST_NO_EXCEPTIONS
    BOOST_CHECK(m.failure());
    try
    {
      boost::rethrow_exception(m.failure());
    }
    catch(const boost::system::system_error &ex)
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
    auto e = boost::copy_exception(Foo());
    outcome<int> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
    BOOST_CHECK_THROW(m.value(), Foo);
    BOOST_CHECK_THROW(m.error(), bad_outcome_access);
    BOOST_CHECK(m.exception() == e);
  }
  {  // outcome<void, void> should work
    boost::system::error_code ec(5, boost::system::system_category());
    auto e = boost::copy_exception(boost::system::system_error(ec));
    outcome<void, void> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(!m.has_error());
    BOOST_CHECK(m.has_exception());
  }
#endif


  {
    outcome<int> a(5);
    outcome<int> b(make_error_code(boost::system::errc::invalid_argument));
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
    catch(const boost::system::system_error & /*unused*/)
    {
    }
#endif
    static_assert(!std::is_default_constructible<decltype(a)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(a)>::value, "");
    static_assert(std::is_copy_constructible<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_copy_constructible<decltype(a)>::value, "");
    static_assert(std::is_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(a)>::value, "");
    static_assert(std::is_nothrow_copy_assignable<decltype(a)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(a)>::value, "");

    // Test void compiles
    outcome<void> c(in_place_type<void>);
    outcome<void> c2(c);
    (void) c2;
    // Test int, void compiles
    outcome<int, void> d(in_place_type<boost::exception_ptr>);
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
    outcome<udt3> i(ENOMEM, boost::system::generic_category());
    BOOST_CHECK(h.has_value());
    BOOST_CHECK(i.has_error());
  }
}
