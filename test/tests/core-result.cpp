/* Unit testing for outcomes
(C) 2013-2025 Niall Douglas <http://www.nedproductions.biz/> (30 commits)


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

#define BOOST_OUTCOME_USE_STD_ADDRESSOF 1

#ifdef TESTING_WG21_EXPERIMENTAL_RESULT
#include <boost/outcome/experimental/result.hpp>
#define BOOST_OUTCOME_AUTO_TEST_CASE(...) BOOST_AUTO_TEST_CASE(__VA_ARGS__)
#else
#include <boost/outcome/result.hpp>
#endif
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_monitor.hpp>

#include <iostream>

#ifndef BOOST_NO_EXCEPTIONS
// Custom error type with payload
struct payload
{
  boost::system::error_code ec;
  const char *str{nullptr};
  payload() = default;
  payload(boost::system::errc::errc_t _ec, const char *_str)
      : ec(make_error_code(_ec))
      , str(_str)
  {
  }
};
struct payload_exception : std::runtime_error
{
  explicit payload_exception(const char *what)
      : std::runtime_error(what)
  {
  }
};
inline const boost::system::error_code &make_error_code(const payload &p)
{
  return p.ec;
}
inline void outcome_throw_as_system_error_with_payload(const payload &p)
{
  throw payload_exception(p.str);
}
#endif

BOOST_OUTCOME_AUTO_TEST_CASE(works_result, "Tests that the result works as intended")
{
#ifdef TESTING_WG21_EXPERIMENTAL_RESULT
  using namespace std::experimental;
  using std::in_place_type;
#else
  using namespace BOOST_OUTCOME_V2_NAMESPACE;
#endif

  static_assert(std::is_constructible<result<long>, int>::value, "Sanity check that monad can be constructed from a value_type");
  static_assert(!std::is_constructible<result<result<long>>, int>::value, "Sanity check that outer monad can be constructed from an inner monad's value_type");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9  // GCCs before 9 barf on this
  static_assert(!std::is_constructible<result<result<result<long>>>, int>::value, "Sanity check that outer monad can be constructed from an inner inner monad's value_type");
  static_assert(!std::is_constructible<result<result<result<result<long>>>>, int>::value, "Sanity check that outer monad can be constructed from an inner inner monad's value_type");
#endif

  static_assert(std::is_constructible<result<int>, result<long>>::value, "Sanity check that compatible monads can be constructed from one another");
  static_assert(std::is_constructible<result<result<int>>, result<long>>::value, "Sanity check that outer monad can be constructed from a compatible monad");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9  // GCCs before 9 barf on this
  static_assert(!std::is_constructible<result<result<result<int>>>, result<long>>::value, "Sanity check that outer monad can be constructed from a compatible monad up to two nestings deep");
  static_assert(!std::is_constructible<result<result<result<result<int>>>>, result<long>>::value, "Sanity check that outer monad can be constructed from a compatible monad three or more nestings deep");
#endif
  static_assert(!std::is_constructible<result<std::string>, result<int>>::value, "Sanity check that incompatible monads cannot be constructed from one another");

#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
  static_assert(std::is_constructible<result<int>, result<void>>::value, "Sanity check that all monads can be constructed from a void monad");
  static_assert(std::is_constructible<result<result<int>>, result<void>>::value, "Sanity check that outer monad can be constructed from a compatible monad");
#if defined(__clang__) || !defined(__GNUC__) || __GNUC__ >= 9  // GCCs before 9 barf on this
  static_assert(std::is_constructible<result<result<result<int>>>, result<void>>::value, "Sanity check that outer monad can be constructed from a compatible monad up to two nestings deep");
#endif
  static_assert(!std::is_constructible<result<void>, result<int>>::value, "Sanity check that incompatible monads cannot be constructed from one another");
#endif
  static_assert(std::is_void<result<void>::value_type>::value, "Sanity check that result<void> has a void value_type");
#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
  static_assert(std::is_void<result<void, void>::error_type>::value, "Sanity check that result<void, void> has a void error_type");
#endif

  static_assert(std::is_same<result<int>::value_type, int>::value, "Sanity check that result<int> has a int value_type");
  static_assert(std::is_same<result<int>::error_type, boost::system::error_code>::value, "Sanity check that result<int> has a error_code error_type");


  {  // errored int
    result<int> m(boost::system::errc::bad_address);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // errored void
    result<void> m(boost::system::errc::bad_address);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
// BOOST_CHECK(!m.has_exception());
#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
    BOOST_CHECK_THROW(([&m]() -> void { return m.value(); }()), boost::system::system_error);
#endif
    BOOST_CHECK_NO_THROW(m.error());
  }
  {  // valued int
    result<int> m(5);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == 5);
    m.value() = 6;
    BOOST_CHECK(m.value() == 6);
    BOOST_CHECK_THROW(m.error(), bad_result_access);
  }
  {  // valued bool
    result<bool> m(false);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == false);
    m.value() = true;
    BOOST_CHECK(m.value() == true);
    BOOST_CHECK_THROW(m.error(), bad_result_access);
  }
  {  // moves do not clear state
    result<std::string> m("niall");
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK(m.value() == "niall");
    m.value() = "NIALL";
    BOOST_CHECK(m.value() == "NIALL");
    auto temp(std::move(m).value());
    BOOST_CHECK(temp == "NIALL");
    BOOST_CHECK(m.value().empty());  // NOLINT
  }
  {  // valued void
    result<void> m(in_place_type<void>);
    BOOST_CHECK(m);
    BOOST_CHECK(m.has_value());
    BOOST_CHECK(!m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_NO_THROW(m.value());  // works, but type returned is unusable
    BOOST_CHECK_THROW(m.error(), bad_result_access);
  }
  {  // errored
    boost::system::error_code ec(5, boost::system::system_category());
    result<int> m(ec);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK(m.error() == ec);
  }
#if !defined(__APPLE__) || defined(__cpp_exceptions)
  {  // errored, custom
    boost::system::error_code ec(5, boost::system::system_category());
    auto e = boost::copy_exception(boost::system::system_error(ec));  // NOLINT
    result<int, boost::exception_ptr> m(e);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    BOOST_CHECK_THROW(m.value(), boost::system::system_error);
    BOOST_CHECK(m.error() == e);
  }
#endif
#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
  {  // custom error type
    struct Foo
    {
    };
    result<int, Foo> m(in_place_type<Foo>);
    BOOST_CHECK(!m);
    BOOST_CHECK(!m.has_value());
    BOOST_CHECK(m.has_error());
    // BOOST_CHECK(!m.has_exception());
    // BOOST_CHECK_NO_THROW(m.value());
    // BOOST_CHECK_NO_THROW(m.error());
  }
  if(false)  // NOLINT
  {          // void, void is permitted, but is not constructible
    result<void, void> *m = nullptr;
    m->value();
    m->error();
  }
#endif

  {
    // Deliberately define non-trivial operations
    struct udt
    {
      int _v{0};
      udt() = default;
      udt(udt &&o) noexcept : _v(o._v) {}
      udt(const udt &o)  // NOLINT
      : _v(o._v)
      {
      }
      udt &operator=(udt &&o) noexcept
      {
        _v = o._v;
        return *this;
      }
      udt &operator=(const udt &o)  // NOLINT
      {
        _v = o._v;
        return *this;
      }
      ~udt() { _v = 0; }
    };
    // No default construction, no copy nor move
    struct udt2
    {
      udt2() = delete;
      udt2(udt2 &&) = delete;
      udt2(const udt2 &) = delete;
      udt2 &operator=(udt2 &&) = delete;
      udt2 &operator=(const udt2 &) = delete;
      explicit udt2(int /*unused*/) {}
      ~udt2() = default;
    };
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
    // Trivial with custom operator&
    struct udt4
    {
      int _v{0};
      udt4() = default;
      udt4(udt4 &&) = default;
      udt4(const udt4 &) = default;
      udt4 &operator=(udt4 &&) = default;
      udt4 &operator=(const udt4 &) = default;
      void operator&() {}
      ~udt4() = default;
    };
    // Non-trivial with custom operator&
    struct udt5
    {
      int _v{0};
      udt5() = default;
      udt5(int v) : _v(v) {}
      udt5(udt5 &&o) noexcept : _v(o._v) {}
      udt5(const udt5 &o)  // NOLINT
      : _v(o._v)
      {
      }
      udt5 &operator=(udt5 &&o) noexcept
      {
        _v = o._v;
        return *this;
      }
      udt5 &operator=(const udt5 &o)  // NOLINT
      {
        _v = o._v;
        return *this;
      }
      void operator&() {}
      ~udt5() { _v = 0; }
    };


    result<int> a(5);
    result<int> b(make_error_code(boost::system::errc::invalid_argument));
    std::cout << sizeof(a) << std::endl;  // 32 bytes
    if(false)                             // NOLINT
    {
      b.assume_value();
      a.assume_error();
    }
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
// Quality of implementation of std::optional is poor :(
#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
    static_assert(std::is_trivially_copy_constructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_copy_constructible<decltype(a)>::value, "");
    static_assert(std::is_copy_assignable<decltype(a)>::value, "");
    static_assert(std::is_trivially_copy_assignable<decltype(a)>::value, "");
    static_assert(std::is_nothrow_copy_assignable<decltype(a)>::value, "");
#endif
    static_assert(std::is_trivially_destructible<decltype(a)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(a)>::value, "");

    // Test void compiles
    result<void> c(in_place_type<void>);
    result<void> c2(c);
    (void) c2;

    // Test a standard udt compiles
    result<udt> d(in_place_type<udt>);
    result<udt> d2(d);
    static_assert(!std::is_default_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(d)>::value, "");
    static_assert(std::is_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(d)>::value, "");
    static_assert(std::is_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(d)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(d)>::value, "");
    static_assert(std::is_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(d)>::value, "");
    static_assert(std::is_nothrow_move_assignable<decltype(d)>::value, "");
    static_assert(!std::is_trivially_destructible<decltype(d)>::value, "");
    static_assert(std::is_nothrow_destructible<decltype(d)>::value, "");

    // Test a highly pathological udt compiles
    result<udt2> e(in_place_type<udt2>, 5);
    // result<udt2> e2(e);
    static_assert(!std::is_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_default_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_constructible<decltype(e)>::value, "");
    static_assert(!std::is_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_copy_assignable<decltype(e)>::value, "");
    static_assert(!std::is_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_trivially_move_assignable<decltype(e)>::value, "");
    static_assert(!std::is_nothrow_move_assignable<decltype(e)>::value, "");

    // Test a udt which can only be constructed in place compiles
    result<udt3> g(in_place_type<udt3>, 5, static_cast<const char *>("niall"), nullptr);
    // Does converting inplace construction also work?
    result<udt3> h(5, static_cast<const char *>("niall"), nullptr);
    result<udt3> i(ENOMEM, boost::system::generic_category());
    BOOST_CHECK(h.has_value());
    BOOST_CHECK(i.has_error());

    // Test udt with custom operator&
    udt4 j0;
    result<udt4> j1(in_place_type<udt4>);
    result<udt4> j2(j0);
    result<udt4> j3(j1);
    result<udt4> j4(std::move(j0));
    result<udt4> j5(std::move(j1));
    (void) j3;
    (void) j5;
    j2 = j0;
    j2 = j1;
    j2 = std::move(j0);
    j2 = std::move(j1);
    j1.swap(j2);

    udt5 k0;
    result<udt5> k1(in_place_type<udt5>);
    result<udt5> k2(k0);
    result<udt5> k3(k1);
    result<udt5> k4(std::move(k0));
    result<udt5> k5(std::move(k1));
    (void) k3;
    (void) k5;
    k2 = k0;
    k2 = k1;
    k2 = std::move(k0);
    k2 = std::move(k1);
    k1.swap(k2);

    result<void> k6(in_place_type<void>);
    result<udt5> k7(k6);
    result<udt5> k8(std::move(k6));

    result<int, void> k9(in_place_type<int>);
    result<udt5, int> k10(k9);
    result<udt5, int> k11(std::move(k9));
  }

  // Test direct use of error code enum works
  {
    constexpr result<int, boost::system::errc::errc_t> a(5), b(boost::system::errc::invalid_argument);
    static_assert(a.value() == 5, "a is not 5");
    static_assert(b.error() == boost::system::errc::invalid_argument, "b is not errored");
    BOOST_CHECK_THROW(b.value(), boost::system::system_error);
  }

#ifndef TESTING_WG21_EXPERIMENTAL_RESULT
#ifndef BOOST_NO_EXCEPTIONS
  // Test payload facility
  {
    const char *niall = "niall";
    result<int, payload> b{boost::system::errc::invalid_argument, niall};
    try
    {
      b.value();
      BOOST_CHECK(false);
    }
    catch(const payload_exception &e)
    {
      BOOST_CHECK(!strcmp(e.what(), niall));
    }
    catch(...)
    {
      BOOST_CHECK(false);
    }
  }
#endif
#endif
}
