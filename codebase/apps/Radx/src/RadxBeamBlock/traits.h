// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Rainfields Utilities Library (rainutil)
// ** Copyright BOM (C) 2013
// ** Bureau of Meteorology, Commonwealth of Australia, 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from the BOM.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of the BOM nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%

#ifndef RAINUTIL_TRAITS_H
#define RAINUTIL_TRAITS_H

#include <cstddef>
#include <type_traits>
#include <utility>
#include <tr2/type_traits>

namespace rainfields
{
  /// get the value of the sizeof() operator for type T
  template <typename T>
  struct size_of : std::integral_constant<decltype(sizeof(T)), sizeof(T)>
  { };

  /// get type of first type in parameter pack
  template <typename F, typename... T>
  struct first_type{ typedef F type; };

  /// get the index of the first occurence of a type in a parameter pack
  template <typename S, typename F, typename... T>
  struct type_index : std::integral_constant<int, type_index<S, T...>::value + 1>
  { };
  template <typename S, typename... T>
  struct type_index<S, S, T...> : std::integral_constant<int, 0>
  { };

  /// trait used determine the minimum integral value of a trait applied to multiple types
  template <template<class> class Trait, typename F, typename... T>
  struct trait_min : std::integral_constant<decltype(Trait<F>::value), (Trait<F>::value < trait_min<Trait, T...>::value) ? Trait<F>::value : trait_min<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_min<Trait, F> : std::integral_constant<decltype(Trait<F>::value), Trait<F>::value>
  { };

  /// trait used determine the maximum integral value of a trait applied to multiple types
  template <template<class> class Trait, typename F, typename... T>
  struct trait_max : std::integral_constant<decltype(Trait<F>::value), (Trait<F>::value > trait_max<Trait, T...>::value) ? Trait<F>::value : trait_max<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_max<Trait, F> : std::integral_constant<decltype(Trait<F>::value), Trait<F>::value>
  { };

  /// true if given trait (Trait) is true when applied to every one of the supplied type (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_all : std::integral_constant<bool, Trait<F>::value && trait_all<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_all<Trait, F> : std::integral_constant<bool, Trait<F>::value>
  { };

  /// true if given trait (Trait) is false when applied to every one of the supplied types (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_none : std::integral_constant<bool, !Trait<F>::value && trait_none<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_none<Trait, F> : std::integral_constant<bool, !Trait<F>::value>
  { };

  /// true if given trait (Trait) is true when applied to any of the supplied types (T...)
  template <template<class> class Trait, typename F, typename... T>
  struct trait_any : std::integral_constant<bool, Trait<F>::value || trait_any<Trait, T...>::value>
  { };
  template <template<class> class Trait, typename F>
  struct trait_any<Trait, F> : std::integral_constant<bool, Trait<F>::value>
  { };

  /// trait used to serialize enumerations (requires specialization)
  template <typename T>
  struct enum_traits
  {
    static_assert(std::is_enum<T>::value, "attempt to instanciate enum_traits on non-enum type");

    static constexpr const char* name = nullptr;
    static constexpr int count = 0;
    static constexpr const char* strings[] = { };
  };

  namespace detail
  {
    // used by is_instance_of
    template <template <typename...> class F>
    struct conversion_tester
    {
      template <typename... Args>
      conversion_tester(const F<Args...>&);
    };

    template <typename T>
    struct void_type{ typedef void type; };
  }

  /// determine if a class is an instance of a template
  template <class C, template <typename...> class T>
  struct is_instance_of
  {
    static const bool value 
      = std::is_convertible<C, detail::conversion_tester<T>>::value;
  };

  /// determine the most 'inner' value_type typedef of a class
  template <class T, typename = void>
  struct inner_value_type
  {
    typedef T type;
  };
  template <class T>
  struct inner_value_type<T, typename detail::void_type<typename T::value_type>::type>
  {
    typedef typename inner_value_type<typename T::value_type>::type type;
  };

  /// get the 'rank' value of a class or 1 if a scalar type is used
#if 0
  template <typename T, bool = std::is_scalar<T>::value>
  struct rank_or_1
  {
    static constexpr size_t value = 1;
  };
  template <typename T>
  struct rank_or_1<T, false>
  {
    static constexpr size_t value = T::rank;
  };
#endif
  template <typename T, typename = void>
  struct rank_or_1
  {
    static constexpr size_t value = 1;
  };
  template <typename T>
  struct rank_or_1<T, decltype(T::rank)>
  {
    static constexpr size_t value = T::rank;
  };

  /// generic typelist implementation
  template <typename...>
  struct typelist;

  template <>
  struct typelist<>
  {
    typedef std::true_type empty;
  };

  template <typename First, typename... Rest>
  struct typelist<First, Rest...>
  {
    typedef std::false_type empty;

    struct first
    {
      typedef First type;
    };

    struct rest
    {
      typedef typelist<Rest...> type;
    };
  };

  /// determine whether a type is a member of a typelist
  template <typename List, typename T, bool empty = List::empty::value>
  struct typelist_contains : std::integral_constant<bool, std::is_same<typename List::first::type, T>::value ? true : typelist_contains<typename List::rest::type, T>::value>
  { };
  template <typename List, typename T>
  struct typelist_contains<List, T, true> : std::false_type
  { };

  /// generate a typelist of all base classes of a type
  template <typename T>
  struct bases
  {
    typedef typename std::tr2::bases<T>::type type;
  };

  // TODO - stuff below here are not technically traits, but usefull function objects or templates
  //        that don't seem to belong elsewhere yet

  /// function object used to call the subscript operator on an object
  struct subscript
  {
    template <typename T, typename K>
    inline auto operator()(T&& obj, K&& key) const -> decltype(std::forward<T>(obj)[std::forward<K>(key)])
    {
      return std::forward<T>(obj)[std::forward<K>(key)];
    }
  };

  /// generic enumerate with the specified number of members
  template <int count> struct generic_enum { };
  template <> struct generic_enum<1>  { enum type { _0 }; };
  template <> struct generic_enum<2>  { enum type { _0, _1 }; };
  template <> struct generic_enum<3>  { enum type { _0, _1, _2 }; };
  template <> struct generic_enum<4>  { enum type { _0, _1, _2, _3 }; };
  template <> struct generic_enum<5>  { enum type { _0, _1, _2, _3, _4 }; };
  template <> struct generic_enum<6>  { enum type { _0, _1, _2, _3, _4, _5 }; };
  template <> struct generic_enum<7>  { enum type { _0, _1, _2, _3, _4, _5, _6 }; };
  template <> struct generic_enum<8>  { enum type { _0, _1, _2, _3, _4, _5, _6, _7 }; };
  template <> struct generic_enum<9>  { enum type { _0, _1, _2, _3, _4, _5, _6, _7, _8 }; };
  template <> struct generic_enum<10> { enum type { _0, _1, _2, _3, _4, _5, _6, _7, _8, _9 }; };
}

#endif
