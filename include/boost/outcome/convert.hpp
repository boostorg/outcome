/* Says how to convert value, error and exception types
(C) 2017-2025 Niall Douglas <http://www.nedproductions.biz/> (12 commits)
File Created: Nov 2017


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

#ifndef BOOST_OUTCOME_CONVERT_HPP
#define BOOST_OUTCOME_CONVERT_HPP

#include "detail/basic_result_storage.hpp"

BOOST_OUTCOME_V2_NAMESPACE_EXPORT_BEGIN

namespace concepts
{
#if defined(__cpp_concepts)
#if(defined(_MSC_VER) || defined(__clang__) || (defined(__GNUC__) && __cpp_concepts >= 201707) || BOOST_OUTCOME_FORCE_STD_BOOST_OUTCOME_C_CONCEPTS) &&                           \
!BOOST_OUTCOME_FORCE_LEGACY_GCC_BOOST_OUTCOME_C_CONCEPTS
#define BOOST_OUTCOME_GCC6_CONCEPT_BOOL
#else
#ifndef BOOST_OUTCOME_SUPPRESS_LEGACY_CONCEPTS_WARNING
#warning "WARNING: Legacy GCC concepts are known to fail to compile in a number of important situations!"
#endif
#define BOOST_OUTCOME_GCC6_CONCEPT_BOOL bool
#endif
  namespace detail
  {
    template <class T, class U>
    concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL SameHelper = std::is_same<T, U>::value;
    template <class T, class U>
    concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL same_as = detail::SameHelper<T, U> && detail::SameHelper<U, T>;
    template <class T, class U>
    concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL convertible = std::is_convertible<T, U>::value;
    template <class T, class U>
    concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL base_of = std::is_base_of<T, U>::value;
  }  // namespace detail


  /* The `value_or_none` concept.
  \requires That `U::value_type` exists and that `std::declval<U>().has_value()` returns a `bool` and `std::declval<U>().value()` exists.
  */
  template <class U>
  concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL value_or_none = requires(U a) {
    {
      a.has_value()
    } -> detail::same_as<bool>;
    {
      a.value()
    };
  };
  /* The `value_or_error` concept.
  \requires That `U::value_type` and `U::error_type` exist;
  that `std::declval<U>().has_value()` returns a `bool`, `std::declval<U>().value()` and  `std::declval<U>().error()` exists.
  */
  template <class U>
  concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL value_or_error = requires(U a) {
    {
      a.has_value()
    } -> detail::same_as<bool>;
    {
      a.value()
    };
    {
      a.error()
    };
  };

#else
  namespace detail
  {
    struct no_match
    {
    };
    inline no_match match_value_or_none(...);
    inline no_match match_value_or_error(...);
    BOOST_OUTCOME_TEMPLATE(class U)
    BOOST_OUTCOME_TREQUIRES(BOOST_OUTCOME_TEXPR(std::declval<U>().has_value()), BOOST_OUTCOME_TEXPR(std::declval<U>().value()))
    inline U match_value_or_none(U &&);
    BOOST_OUTCOME_TEMPLATE(class U)
    BOOST_OUTCOME_TREQUIRES(BOOST_OUTCOME_TEXPR(std::declval<U>().has_value()), BOOST_OUTCOME_TEXPR(std::declval<U>().value()), BOOST_OUTCOME_TEXPR(std::declval<U>().error()))
    inline U match_value_or_error(U &&);

    template <class U>
    static constexpr bool value_or_none =
    !std::is_same<no_match, decltype(match_value_or_none(std::declval<BOOST_OUTCOME_V2_NAMESPACE::detail::devoid<U>>()))>::value;
    template <class U>
    static constexpr bool value_or_error =
    !std::is_same<no_match, decltype(match_value_or_error(std::declval<BOOST_OUTCOME_V2_NAMESPACE::detail::devoid<U>>()))>::value;
  }  // namespace detail
  /* The `value_or_none` concept.
  \requires That `U::value_type` exists and that `std::declval<U>().has_value()` returns a `bool` and `std::declval<U>().value()` exists.
  */
  template <class U> static constexpr bool value_or_none = detail::value_or_none<U>;
  /* The `value_or_error` concept.
  \requires That `U::value_type` and `U::error_type` exist;
  that `std::declval<U>().has_value()` returns a `bool`, `std::declval<U>().value()` and  `std::declval<U>().error()` exists.
  */
  template <class U> static constexpr bool value_or_error = detail::value_or_error<U>;
#endif
}  // namespace concepts

namespace convert
{
#if BOOST_OUTCOME_ENABLE_LEGACY_SUPPORT_FOR < 220
#if defined(__cpp_concepts)
  template <class U>
  concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL ValueOrNone = concepts::value_or_none<U>;
  template <class U>
  concept BOOST_OUTCOME_GCC6_CONCEPT_BOOL ValueOrError = concepts::value_or_error<U>;
#else
  template <class U> static constexpr bool ValueOrNone = concepts::value_or_none<U>;
  template <class U> static constexpr bool ValueOrError = concepts::value_or_error<U>;
#endif
#endif

  namespace detail
  {
    template <class T, class X> struct make_type
    {
      template <class U> static constexpr T value(U &&v) { return T{in_place_type<typename T::value_type>, static_cast<U &&>(v).value()}; }
      template <class U> static constexpr T error(U &&v) { return T{in_place_type<typename T::error_type>, static_cast<U &&>(v).error()}; }
      static constexpr T error() { return T{in_place_type<typename T::error_type>}; }
    };
    template <class T> struct make_type<T, void>
    {
      template <class U> static constexpr T value(U && /*unused*/) { return T{in_place_type<typename T::value_type>}; }
      template <class U> static constexpr T error(U && /*unused*/) { return T{in_place_type<typename T::error_type>}; }
      static constexpr T error() { return T{in_place_type<typename T::error_type>}; }
    };
  }  // namespace detail

  /*! AWAITING HUGO JSON CONVERSION TOOL
type definition  value_or_error. Potential doc page: NOT FOUND
*/
  template <class T, class U> struct value_or_error
  {
    static constexpr bool enable_result_inputs = false;
    static constexpr bool enable_outcome_inputs = false;
    BOOST_OUTCOME_TEMPLATE(class X)
    BOOST_OUTCOME_TREQUIRES(
    BOOST_OUTCOME_TPRED(std::is_same<U, std::decay_t<X>>::value                                                                                       //
                  &&concepts::value_or_error<U>                                                                                                 //
                  && (std::is_void<typename std::decay_t<X>::value_type>::value ||
                      BOOST_OUTCOME_V2_NAMESPACE::detail::is_explicitly_constructible<typename T::value_type, typename std::decay_t<X>::value_type>)  //
                  &&(std::is_void<typename std::decay_t<X>::error_type>::value ||
                     BOOST_OUTCOME_V2_NAMESPACE::detail::is_explicitly_constructible<typename T::error_type, typename std::decay_t<X>::error_type>) ))
    constexpr T operator()(X &&v)
    {
      return v.has_value() ? detail::make_type<T, typename T::value_type>::value(static_cast<X &&>(v)) :
                             detail::make_type<T, typename U::error_type>::error(static_cast<X &&>(v));
    }
  };
}  // namespace convert

BOOST_OUTCOME_V2_NAMESPACE_END

#endif
