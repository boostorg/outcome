/* Traits for Outcome
(C) 2018-2025 Niall Douglas <http://www.nedproductions.biz/> (8 commits)
File Created: March 2018


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

#ifndef BOOST_OUTCOME_TRAIT_HPP
#define BOOST_OUTCOME_TRAIT_HPP

#include "config.hpp"

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace trait
{
  /*! AWAITING HUGO JSON CONVERSION TOOL
SIGNATURE NOT RECOGNISED
*/
  template <class R>                                                             //
  static constexpr bool type_can_be_used_in_basic_result =                       //
  (!std::is_reference<R>::value                                                  //
   && !BOOST_OUTCOME_V2_NAMESPACE::detail::is_in_place_type_t<std::decay_t<R>>::value  //
   && !is_success_type<R>                                                        //
   && !is_failure_type<R>                                                        //
   && !std::is_array<R>::value                                                   //
   && (std::is_void<R>::value || (std::is_object<R>::value                       //
                                  && std::is_destructible<R>::value))            //
  );

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  is_error_type. Potential doc page: NOT FOUND
*/
  template <class T> struct is_move_bitcopying
  {
    static constexpr bool value = false;
  };

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  is_error_type. Potential doc page: NOT FOUND
*/
  template <class E> struct is_error_type
  {
    static constexpr bool value = false;
  };

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  is_error_type_enum. Potential doc page: NOT FOUND
*/
  template <class E, class Enum> struct is_error_type_enum
  {
    static constexpr bool value = false;
  };

  namespace detail
  {
    template <class T> using devoid = BOOST_OUTCOME_V2_NAMESPACE::detail::devoid<T>;
    template <class T> std::add_rvalue_reference_t<devoid<T>> declval() noexcept;

    // From http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4436.pdf
    namespace detector_impl
    {
      template <class...> using void_t = void;
      template <class Default, class, template <class...> class Op, class... Args> struct detector
      {
        static constexpr bool value = false;
        using type = Default;
      };
      template <class Default, template <class...> class Op, class... Args> struct detector<Default, void_t<Op<Args...>>, Op, Args...>
      {
        static constexpr bool value = true;
        using type = Op<Args...>;
      };
    }  // namespace detector_impl
    template <template <class...> class Op, class... Args> using is_detected = detector_impl::detector<void, void, Op, Args...>;

    template <class Arg> using result_of_make_error_code = decltype(make_error_code(declval<Arg>()));
    template <class Arg> using introspect_make_error_code = is_detected<result_of_make_error_code, Arg>;

    template <class Arg> using result_of_make_exception_ptr = decltype(make_exception_ptr(declval<Arg>()));
    template <class Arg> using introspect_make_exception_ptr = is_detected<result_of_make_exception_ptr, Arg>;

    template <class T> struct _is_error_code_available
    {
      static constexpr bool value = detail::introspect_make_error_code<T>::value;
      using type = typename detail::introspect_make_error_code<T>::type;
    };
    template <class T> struct _is_exception_ptr_available
    {
      static constexpr bool value = detail::introspect_make_exception_ptr<T>::value;
      using type = typename detail::introspect_make_exception_ptr<T>::type;
    };
  }  // namespace detail

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  is_error_code_available. Potential doc page: NOT FOUND
*/
  template <class T> struct is_error_code_available
  {
    static constexpr bool value = detail::_is_error_code_available<std::decay_t<T>>::value;
    using type = typename detail::_is_error_code_available<std::decay_t<T>>::type;
  };
  template <class T> constexpr bool is_error_code_available_v = detail::_is_error_code_available<std::decay_t<T>>::value;

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  is_exception_ptr_available. Potential doc page: NOT FOUND
*/
  template <class T> struct is_exception_ptr_available
  {
    static constexpr bool value = detail::_is_exception_ptr_available<std::decay_t<T>>::value;
    using type = typename detail::_is_exception_ptr_available<std::decay_t<T>>::type;
  };
  template <class T> constexpr bool is_exception_ptr_available_v = detail::_is_exception_ptr_available<std::decay_t<T>>::value;


}  // namespace trait

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
