/* Unit testing for outcomes
(C) 2013-2025 Niall Douglas <http://www.nedproductions.biz/> (1 commit)


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

BOOST_OUTCOME_AUTO_TEST_CASE(issues_0115_outcome, "Initialization from `failure_type` drops default-constructed values")
{
  namespace out = BOOST_OUTCOME_V2_NAMESPACE;

  out::outcome<int> o1 = boost::system::error_code{};
  BOOST_CHECK(o1.has_error());
  BOOST_CHECK(!o1.has_exception());

  out::outcome<int> o2 = out::failure(boost::system::error_code{});
  BOOST_CHECK(o2.has_error());
  BOOST_CHECK(!o2.has_exception());

  out::outcome<int> o3(boost::system::error_code{}, boost::exception_ptr{});
  BOOST_CHECK(o3.has_error());
  BOOST_CHECK(o3.has_exception());

  out::outcome<int> o4 = out::failure(boost::system::error_code{}, boost::exception_ptr{});
  BOOST_CHECK(o4.has_error());
  BOOST_CHECK(o4.has_exception());

  out::outcome<int> o5 = out::failure(boost::exception_ptr{});
  BOOST_CHECK(!o5.has_error());
  BOOST_CHECK(o5.has_exception());
}
