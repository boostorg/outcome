/* Storage for a very simple basic_result type
(C) 2017-2019 Niall Douglas <http://www.nedproductions.biz/> (6 commits)
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

#ifndef BOOST_OUTCOME_BASIC_RESULT_STORAGE_HPP
#define BOOST_OUTCOME_BASIC_RESULT_STORAGE_HPP

#include "../success_failure.hpp"
#include "../trait.hpp"
#include "value_storage.hpp"

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace detail
{
  template <class State, class E> constexpr inline void _set_error_is_errno(State & /*unused*/, const E & /*unused*/) {}
  template <class R, class S, class NoValuePolicy> class basic_result_final;
}  // namespace detail

namespace hooks
{
  template <class R, class S, class NoValuePolicy> constexpr inline uint16_t spare_storage(const detail::basic_result_final<R, S, NoValuePolicy> *r) noexcept;
  template <class R, class S, class NoValuePolicy> constexpr inline void set_spare_storage(detail::basic_result_final<R, S, NoValuePolicy> *r, uint16_t v) noexcept;
}  // namespace hooks

namespace policy
{
  struct base;
}  // namespace policy

namespace detail
{
  template <bool value_throws, bool error_throws> struct basic_result_storage_swap;
  template <class R, class EC, class NoValuePolicy>                                                                                                                                    //
  BOOST_OUTCOME_REQUIRES(trait::type_can_be_used_in_basic_result<R> &&trait::type_can_be_used_in_basic_result<EC> && (std::is_void<EC>::value || std::is_default_constructible<EC>::value))  //
  class basic_result_storage
  {
    static_assert(trait::type_can_be_used_in_basic_result<R>, "The type R cannot be used in a basic_result");
    static_assert(trait::type_can_be_used_in_basic_result<EC>, "The type S cannot be used in a basic_result");
    static_assert(std::is_void<EC>::value || std::is_default_constructible<EC>::value, "The type S must be void or default constructible");

    friend struct policy::base;
    template <class T, class U, class V>                                                                                                                                              //
    BOOST_OUTCOME_REQUIRES(trait::type_can_be_used_in_basic_result<T> &&trait::type_can_be_used_in_basic_result<U> && (std::is_void<U>::value || std::is_default_constructible<U>::value))  //
    friend class basic_result_storage;
    template <class T, class U, class V> friend class basic_result_final;
    template <class T, class U, class V> friend constexpr inline uint16_t hooks::spare_storage(const detail::basic_result_final<T, U, V> *r) noexcept;        // NOLINT
    template <class T, class U, class V> friend constexpr inline void hooks::set_spare_storage(detail::basic_result_final<T, U, V> *r, uint16_t v) noexcept;  // NOLINT
    template <bool value_throws, bool error_throws> struct basic_result_storage_swap;

    struct disable_in_place_value_type
    {
    };
    struct disable_in_place_error_type
    {
    };

  protected:
    using _value_type = std::conditional_t<std::is_same<R, EC>::value, disable_in_place_value_type, R>;
    using _error_type = std::conditional_t<std::is_same<R, EC>::value, disable_in_place_error_type, EC>;

#ifdef BOOST_OUTCOME_STANDARDESE_IS_IN_THE_HOUSE
    detail::value_storage_trivial<_value_type> _state;
#else
    detail::value_storage_select_impl<_value_type> _state;
#endif
    detail::devoid<_error_type> _error;

  public:
    // Used by iostream support to access state
    detail::value_storage_select_impl<_value_type> &_iostreams_state() { return _state; }
    const detail::value_storage_select_impl<_value_type> &_iostreams_state() const { return _state; }

    // Hack to work around MSVC bug in /permissive-
    detail::value_storage_select_impl<_value_type> &_msvc_nonpermissive_state() { return _state; }
    detail::devoid<_error_type> &_msvc_nonpermissive_error() { return _error; }

  protected:
    basic_result_storage() = default;
    basic_result_storage(const basic_result_storage &) = default;             // NOLINT
    basic_result_storage(basic_result_storage &&) = default;                  // NOLINT
    basic_result_storage &operator=(const basic_result_storage &) = default;  // NOLINT
    basic_result_storage &operator=(basic_result_storage &&) = default;       // NOLINT
    ~basic_result_storage() = default;

    template <class... Args>
    constexpr explicit basic_result_storage(in_place_type_t<_value_type> _, Args &&... args) noexcept(std::is_nothrow_constructible<_value_type, Args...>::value)
        : _state{_, static_cast<Args &&>(args)...}
        , _error()
    {
    }
    template <class U, class... Args>
    constexpr basic_result_storage(in_place_type_t<_value_type> _, std::initializer_list<U> il, Args &&... args) noexcept(std::is_nothrow_constructible<_value_type, std::initializer_list<U>, Args...>::value)
        : _state{_, il, static_cast<Args &&>(args)...}
        , _error()
    {
    }
    template <class... Args>
    constexpr explicit basic_result_storage(in_place_type_t<_error_type> /*unused*/, Args &&... args) noexcept(std::is_nothrow_constructible<_error_type, Args...>::value)
        : _state{detail::status_have_error}
        , _error(static_cast<Args &&>(args)...)
    {
      _set_error_is_errno(_state, _error);
    }
    template <class U, class... Args>
    constexpr basic_result_storage(in_place_type_t<_error_type> /*unused*/, std::initializer_list<U> il, Args &&... args) noexcept(std::is_nothrow_constructible<_error_type, std::initializer_list<U>, Args...>::value)
        : _state{detail::status_have_error}
        , _error{il, static_cast<Args &&>(args)...}
    {
      _set_error_is_errno(_state, _error);
    }
    struct compatible_conversion_tag
    {
    };
    template <class T, class U, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, const basic_result_storage<T, U, V> &o) noexcept(std::is_nothrow_constructible<_value_type, T>::value &&std::is_nothrow_constructible<_error_type, U>::value)
        : _state(o._state)
        , _error(o._error)
    {
    }
    template <class T, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, const basic_result_storage<T, void, V> &o) noexcept(std::is_nothrow_constructible<_value_type, T>::value)
        : _state(o._state)
        , _error(_error_type{})
    {
    }
    template <class T, class U, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, basic_result_storage<T, U, V> &&o) noexcept(std::is_nothrow_constructible<_value_type, T>::value &&std::is_nothrow_constructible<_error_type, U>::value)
        : _state(static_cast<decltype(o._state) &&>(o._state))
        , _error(static_cast<U &&>(o._error))
    {
    }
    template <class T, class V>
    constexpr basic_result_storage(compatible_conversion_tag /*unused*/, basic_result_storage<T, void, V> &&o) noexcept(std::is_nothrow_constructible<_value_type, T>::value)
        : _state(static_cast<decltype(o._state) &&>(o._state))
        , _error(_error_type{})
    {
    }
  };

// Neither value nor error type can throw during swap
#ifndef BOOST_NO_EXCEPTIONS
  template <> struct basic_result_storage_swap<false, false>
#else
  template <bool value_throws, bool error_throws> struct basic_result_storage_swap
#endif
  {
    template <class R, class EC, class NoValuePolicy> constexpr basic_result_storage_swap(basic_result_storage<R, EC, NoValuePolicy> &a, basic_result_storage<R, EC, NoValuePolicy> &b)
    {
      using std::swap;
      a._msvc_nonpermissive_state().swap(b._msvc_nonpermissive_state());
      swap(a._msvc_nonpermissive_error(), b._msvc_nonpermissive_error());
    }
  };
#ifndef BOOST_NO_EXCEPTIONS
  // Swap potentially throwing value first
  template <> struct basic_result_storage_swap<true, false>
  {
    template <class R, class EC, class NoValuePolicy> constexpr basic_result_storage_swap(basic_result_storage<R, EC, NoValuePolicy> &a, basic_result_storage<R, EC, NoValuePolicy> &b)
    {
      using std::swap;
      a._msvc_nonpermissive_state().swap(b._msvc_nonpermissive_state());
      swap(a._msvc_nonpermissive_error(), b._msvc_nonpermissive_error());
    }
  };
  // Swap potentially throwing error first
  template <> struct basic_result_storage_swap<false, true>
  {
    template <class R, class EC, class NoValuePolicy> constexpr basic_result_storage_swap(basic_result_storage<R, EC, NoValuePolicy> &a, basic_result_storage<R, EC, NoValuePolicy> &b)
    {
      struct _
      {
        unsigned &a, &b;
        bool all_good{false};
        ~_()
        {
          if(!all_good)
          {
            // We lost one of the values
            a |= status_lost_consistency;
            b |= status_lost_consistency;
          }
        }
      } _{a._msvc_nonpermissive_state()._status, b._msvc_nonpermissive_state()._status};
      strong_swap(_.all_good, a._msvc_nonpermissive_error(), b._msvc_nonpermissive_error());
      a._msvc_nonpermissive_state().swap(b._msvc_nonpermissive_state());
    }
  };
  // Both could throw
  template <> struct basic_result_storage_swap<true, true>
  {
    template <class R, class EC, class NoValuePolicy> basic_result_storage_swap(basic_result_storage<R, EC, NoValuePolicy> &a, basic_result_storage<R, EC, NoValuePolicy> &b)
    {
      using std::swap;
      // Swap value and status first, if it throws, status will remain unchanged
      a._msvc_nonpermissive_state().swap(b._msvc_nonpermissive_state());
      bool all_good = false;
      try
      {
        strong_swap(all_good, a._msvc_nonpermissive_error(), b._msvc_nonpermissive_error());
      }
      catch(...)
      {
        if(!all_good)
        {
          a._msvc_nonpermissive_state()._status |= detail::status_lost_consistency;
          b._msvc_nonpermissive_state()._status |= detail::status_lost_consistency;
        }
        else
        {
          // We may still be able to rescue tis
          // First try to put the value and status back
          try
          {
            a._msvc_nonpermissive_state().swap(b._msvc_nonpermissive_state());
            // If that succeeded, continue by rethrowing the exception
          }
          catch(...)
          {
            all_good = false;
          }
        }
        if(!all_good)
        {
          // We are now trapped. The value swapped, the error did not,
          // trying to restore the value failed. We now have
          // inconsistent result objects. Best we can do is fix up the
          // status bits to prevent has_value() == has_error().
          auto check = [](basic_result_storage<R, EC, NoValuePolicy> &x) {
            bool has_value = (x._state._status & detail::status_have_value) != 0;
            bool has_error = (x._state._status & detail::status_have_error) != 0;
            bool has_exception = (x._state._status & detail::status_have_exception) != 0;
            x._state._status |= detail::status_lost_consistency;
            if(has_value == (has_error || has_exception))
            {
              if(has_value)
              {
                // We know the value swapped and is now set, so clear error and exception
                x._state._status &= ~(detail::status_have_error | detail::status_have_exception);
              }
              else
              {
                // We know the value swapped and is now unset, so set error
                x._state._status |= detail::status_have_error;
                // TODO: Should I default construct reset _error? It's guaranteed default constructible.
              }
            }
          };
          check(a);
          check(b);
        }
        throw;
      }
    }
  };
#endif

}  // namespace detail
BOOST_OUTCOME_V2_NAMESPACE_END

#endif
