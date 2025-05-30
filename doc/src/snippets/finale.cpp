/* Example of how to marshall Outcomes at namespace boundaries
(C) 2017-2025 Niall Douglas <http://www.nedproductions.biz/> (11 commits)


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

#include "../../../include/boost/outcome.hpp"
#if __has_include("quickcpplib/string_view.hpp")
#include "quickcpplib/string_view.hpp"
#else
#include "../../../include/boost/outcome/quickcpplib/include/quickcpplib/string_view.hpp"
#endif
#include <cstring>  // for memcpy

#if __has_include(<filesystem>) && (__cplusplus >= 201700 || _HAS_CXX17)
#include <filesystem>
namespace filesystem = std::filesystem;
#else
#include <experimental/filesystem>
namespace filesystem = std::experimental::filesystem;
#endif

//! [httplib]
// This is some standalone library implementing high level HTTP
namespace httplib
{
  // These are the error code that this HTTP library can return
  enum class status_code
  {
    success = 0,  // not the HTTP success code of 200

    // A subset of all HTTP status codes for brevity
    bad_request = 400,
    access_denied = 401,
    logon_failed = 402,
    forbidden = 403,
    not_found = 404,
    internal_error = 500
  };
  // This is the error type that this HTTP library can return
  struct failure
  {
    status_code status{status_code::success};
    std::string url{};  // The failing URL
  };
  // Localise a result implementation to this library, holding
  // the logic error of incorrect observation to mean program
  // termination.
  template <class T>
  using result =  //
  BOOST_OUTCOME_V2_NAMESPACE::result<T, failure, BOOST_OUTCOME_V2_NAMESPACE::policy::terminate>;

  /* Performs a HTTP GET on the url, returning the body if successful,
  a failure with status_code if unsuccessful at the HTTP level, or a
  C++ exception throw if something catastrophic happened e.g. bad_alloc
  */
  result<std::string> get(std::string url);
}  // namespace httplib
//! [httplib]

namespace httplib
{
  result<std::string> get(std::string url)
  {
    (void) url;
#if 1
    return "hello world";
#else
    return failure{status_code::not_found, url};
#endif
  }
}  // namespace httplib

namespace filelib
{
  using QUICKCPPLIB_NAMESPACE::string_view::string_view;
  using filesystem::filesystem_error;
  using filesystem::path;
}  // namespace filelib

namespace app
{
  using QUICKCPPLIB_NAMESPACE::string_view::string_view;
}

//! [filelib]
// You may remember this from the tutorial section on Custom Payloads
namespace filelib
{
  // Error code + paths related to a failure. Also causes ADL discovery
  // to check this namespace.
  struct failure_info
  {
    std::error_code ec;
    path path1{}, path2{};
  };

  // Tell Outcome that failure_info is to be treated as a std::error_code
  inline const std::error_code &make_error_code(const failure_info &fi) { return fi.ec; }

  // Tell Outcome that no-value observation should throw a custom exception
  inline void outcome_throw_as_system_error_with_payload(failure_info fi)
  {
    // If the error code is not filesystem related e.g. ENOMEM, throw that
    // as a standard STL exception.
    BOOST_OUTCOME_V2_NAMESPACE::try_throw_std_exception_from_error(fi.ec);
    // Throw the exact same filesystem_error exception which the throwing
    // copy_file() edition does.
    throw filesystem_error(fi.ec.message(), std::move(fi.path1), std::move(fi.path2), fi.ec);
  }

  // Localise a result implementation specific to this namespace.
  template <class T> using result = BOOST_OUTCOME_V2_NAMESPACE::result<T, failure_info>;

  // Writes a chunk of data to some file. Returns bytes written, or
  // failure_info. Never throws exceptions.
  result<size_t> write_file(string_view chunk) noexcept;
}  // namespace filelib
//! [filelib]

namespace filelib
{
  result<size_t> write_file(string_view chunk) noexcept
  {
    (void) chunk;
    return failure_info{make_error_code(std::errc::no_space_on_device), "somepath"};
  }
}  // namespace filelib

//! [tidylib]
// There actually is a library for tidying HTML into XHTML called HTMLTidy
// See http://www.html-tidy.org/
// HTMLTidy is actually a great tool for dealing with 1990s-era tag soup
// HTML, I highly recommend it.

// This isn't the API for Tidy, but let's assume it's a C library returning
// errno domained error codes. out must be freed with free() after use.
extern "C" int tidy_html(char **out, size_t *outlen, const char *in, size_t inlen);
//! [tidylib]

extern "C" int tidy_html(char **out, size_t *outlen, const char *in, size_t inlen)
{
#if 1
  *out = (char *) malloc(inlen + 1);
  memcpy(*out, in, inlen + 1);
  *outlen = inlen;
  return 0;
#else
  // return ENOMEM;
  return EROFS;
#endif
}

//! [app]
// This is the namespace of the application which is connecting together the httplib,
// filelib and tidylib libraries into a solution.
namespace app
{
  // Create an ADL bridge so copy/move hooks will be searched for in this namespace
  struct error_code : public std::error_code
  {
    // passthrough
    using std::error_code::error_code;
    error_code() = default;
    error_code(std::error_code ec)
        : std::error_code(ec)
    {
    }
  };
  // Localise an outcome implementation for this namespace
  template <class T>
  using outcome =  //
  BOOST_OUTCOME_V2_NAMESPACE::outcome<T, error_code /*, std::exception_ptr */>;
  using BOOST_OUTCOME_V2_NAMESPACE::success;
}  // namespace app
//! [app]

//! [app_map_httplib1]
namespace app
{
  // Specialise an exception type for httplib errors
  struct httplib_error : std::runtime_error
  {
    // passthrough
    using std::runtime_error::runtime_error;
    httplib_error(httplib::failure _failure, std::string msg)
        : std::runtime_error(std::move(msg))
        , failure(std::move(_failure))
    {
    }

    // the original failure
    httplib::failure failure;
  };

  // Type erase httplib::result<U> into a httplib_error exception ptr
  template <class U>  //
  inline std::exception_ptr make_httplib_exception(const httplib::result<U> &src)
  {
    std::string str("httplib failed with error ");
    switch(src.error().status)
    {
    case httplib::status_code::success:
      str.append("success");
      break;
    case httplib::status_code::bad_request:
      str.append("bad request");
      break;
    case httplib::status_code::access_denied:
      str.append("access denied");
      break;
    case httplib::status_code::logon_failed:
      str.append("logon failed");
      break;
    case httplib::status_code::forbidden:
      str.append("forbidden");
      break;
    case httplib::status_code::not_found:
      str.append("not found");
      break;
    case httplib::status_code::internal_error:
      str.append("internal error");
      break;
    }
    str.append(" [url was ");
    str.append(src.error().url);
    str.append("]");
    return std::make_exception_ptr(httplib_error(src.error(), std::move(str)));
  }
}  // namespace app
//! [app_map_httplib1]

//! [app_map_httplib2]
// Inject custom ValueOrError conversion
BOOST_OUTCOME_V2_NAMESPACE_BEGIN
namespace convert
{
  // Provide custom ValueOrError conversion from
  // httplib::result<U> into any app::outcome<T>
  template <class T, class U>  //
  struct value_or_error<app::outcome<T>, httplib::result<U>>
  {
    // False to indicate that this converter wants `result`/`outcome`
    // to NOT reject all other `result`
    static constexpr bool enable_result_inputs = true;
    // False to indicate that this converter wants `outcome` to NOT
    // reject all other `outcome`
    static constexpr bool enable_outcome_inputs = true;

    template <class X,                                                                              //
              typename = std::enable_if_t<std::is_same<httplib::result<U>, std::decay_t<X>>::value  //
                                          && std::is_constructible<T, U>::value>>                   //
    constexpr app::outcome<T> operator()(X &&src)
    {
      // Forward any successful value, else synthesise an exception ptr
      return src.has_value() ?                              //
             app::outcome<T>{std::forward<X>(src).value()}  //
             :
             app::outcome<T>{app::make_httplib_exception(std::forward<X>(src))};
    }
  };
}  // namespace convert
BOOST_OUTCOME_V2_NAMESPACE_END
//! [app_map_httplib2]

namespace app
{
  static outcome<int> test_value_or_error2 = BOOST_OUTCOME_V2_NAMESPACE::convert::value_or_error<outcome<int>, httplib::result<int>>{}(httplib::result<int>{5});
  static outcome<int> test_value_or_error3(httplib::result<int>{5});
}  // namespace app

//! [app_map_filelib]
// Inject custom ValueOrError conversion
BOOST_OUTCOME_V2_NAMESPACE_BEGIN
namespace convert
{
  // Provide custom ValueOrError conversion from filelib::result<U>
  // into any app::outcome<T>
  template <class T, class U>  //
  struct value_or_error<app::outcome<T>, filelib::result<U>>
  {
    // True to indicate that this converter wants `result`/`outcome`
    // to NOT reject all other `result`
    static constexpr bool enable_result_inputs = true;
    // False to indicate that this converter wants `outcome` to NOT
    // reject all other `outcome`
    static constexpr bool enable_outcome_inputs = true;

    template <class X,                                                                              //
              typename = std::enable_if_t<std::is_same<filelib::result<U>, std::decay_t<X>>::value  //
                                          && std::is_constructible<T, U>::value>>                   //
    constexpr app::outcome<T> operator()(X &&src)
    {
      // Forward any successful value
      if(src.has_value())
      {
        return {std::forward<X>(src).value()};
      }

      // Synthesise a filesystem_error, exactly as if someone had
      // called src.value()
      auto &fi = src.error();
      BOOST_OUTCOME_V2_NAMESPACE::try_throw_std_exception_from_error(fi.ec);  // might throw
      return {std::make_exception_ptr(                                  //
      filelib::filesystem_error(fi.ec.message(), std::move(fi.path1), std::move(fi.path2), fi.ec))};
    }
  };
}  // namespace convert
BOOST_OUTCOME_V2_NAMESPACE_END
//! [app_map_filelib]

//! [app_map_tidylib]
namespace app
{
  // Specialise an exception type for tidylib errors
  struct tidylib_error : std::system_error
  {
    // passthrough
    using std::system_error::system_error;
    tidylib_error() = default;
    explicit tidylib_error(int c)
        : std::system_error(c, std::generic_category())
    {
    }
  };

  // Create a C++ invoking wrapper for the tidylib C API, modifying data with the returned data,
  // returing a unique_ptr to release storage on scope exit.
  struct call_free
  {
    template <class T> void operator()(T *p) { ::free(p); }
  };
  inline outcome<std::unique_ptr<char, call_free>> tidy_html(string_view &data)
  {
    char *out = nullptr;
    size_t outlen = 0;
    int errcode = ::tidy_html(&out, &outlen, data.data(), data.size());
    if(errcode != 0)
    {
      // If the error code matches a standard STL exception, throw as that.
      BOOST_OUTCOME_V2_NAMESPACE::try_throw_std_exception_from_error(std::error_code(errcode, std::generic_category()));
      // Otherwise wrap the error code into a tidylib_error exception throw
      return std::make_exception_ptr(tidylib_error(errcode));
    }
    // Reset input view to tidied html
    data = string_view(out, outlen);
    // Return a unique ptr to release storage on scope exit
    return std::unique_ptr<char, call_free>(out);
  }
}  // namespace app
//! [app_map_tidylib]

//! [app_go]
namespace app
{
  // A markup function to indicate when we are ValueOrError converting
  template <class T> inline outcome<typename T::value_type> ext(T &&v)
  {  //
    return outcome<typename T::value_type>(std::move(v));
  }

  outcome<void> go()  // NOT noexcept, this can throw STL exceptions e.g. bad_alloc
  {
    // Note that explicit construction is required when converting between differing types
    // of outcome and result. This makes it explicit what you intend to do as conversion
    // may be a lot more expensive than moves.

    // Try to GET this URL. If an unsuccessful HTTP status is returned, serialise a string
    // containing a description of the HTTP status code and the URL which failed, storing
    // that into a httplib_error exception type which is stored as an exception ptr. The
    // TRY operation below will return that exception ptr to be rethrown in the caller.
    // Otherwise the fetched data is returned in a std::string data.
    BOOST_OUTCOME_TRY(auto data, ext(httplib::get("http://www.nedproductions.biz/")));
    string_view data_view(data);

    // HTML tidy the fetched data. If the C library fails due to an error corresponding to
    // a standard library exception type, throw that. Otherwise, synthesise an exception
    // ptr of type tidylib_error which stores the error code returned in an error code with
    // generic category (i.e. errno domain).
    // TRY operation below will return that exception ptr to be rethrown in the caller.
    // Otherwise the tidied data is returned into holdmem, with the string view updated to
    // point at the tidied data.
    BOOST_OUTCOME_TRY(auto holdmem, ext(tidy_html(data_view)));

    // Write the tidied data to some file. If the write fails, synthesise a filesystem_error
    // exception ptr exactly as if one called filelib::write_file(data_view).value().
    BOOST_OUTCOME_TRY(auto written, ext(filelib::write_file(data_view)));
    return success();
  }
}  // namespace app
//! [app_go]

int main()
{
  try
  {
    app::go().value();
  }
  catch(const filelib::filesystem_error &e)
  {
    std::cerr << "Exception thrown, " << e.what()                               //
              << " (path1 = " << e.path1() << ", path2 = " << e.path2() << ")"  //
              << std::endl;
    return 1;
  }
  catch(const std::exception &e)
  {
    std::cerr << "Exception thrown, " << e.what() << std::endl;
    return 1;
  }
  return 0;
}