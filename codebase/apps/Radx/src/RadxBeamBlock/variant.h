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

#ifndef RAINUTIL_VARIANT_H
#define RAINUTIL_VARIANT_H

#include "traits.h"

#include <stdexcept>
#include <utility>

namespace rainfields
{
  namespace detail
  {
    template <typename S>
    static void s_destroy(void* v)                { reinterpret_cast<S*>(v)->~S(); };
    template <typename S>
    static void s_concopy(void* l, const void* r) { new (l) S(*reinterpret_cast<const S*>(r)); }
    template <typename S>
    static void s_conmove(void* l, void* r)       { new (l) S(std::move(*reinterpret_cast<S*>(r))); }
    template <typename S>
    static void s_asscopy(void* l, const void* r) { *reinterpret_cast<S*>(l) = *reinterpret_cast<const S*>(r); }
    template <typename S>
    static void s_assmove(void* l, void* r)       { *reinterpret_cast<S*>(l) = std::move(*reinterpret_cast<S*>(r)); }
  }

  /// variant able to take one of a closed set of specified types
  /**
   * The only requirement imposed on types used with this template is that they be nothrow move
   * constructible (ie: std::is_nothrow_move_constructible<T>::value == true for all T).  Types
   * which are also nothrow copy constructible will also gain a significant performance boost when
   * using copy assignment, or calling set() with a lvalue.
   */
  template <typename... T>
  class variant
  {
  private:
    // assert that all the types are nothrow move constructible
    static_assert(
          trait_all<std::is_nothrow_move_constructible, T...>::value
        , "all types used with variant must be nothrow move constructible");

    // ideally we would use std::aligned_union<0, T...> here instead, but gcc support is lacking
    typedef typename std::aligned_storage<trait_max<size_of, T...>::value, trait_max<std::alignment_of, T...>::value>::type storage;

    // arrays used to lookup generic destruct/copy/move functions for each type
    static constexpr void (*destroy[])(void*) = { &detail::s_destroy<T>... };
    static constexpr void (*copycon[])(void*, const void*) = { &detail::s_concopy<T>... };
    static constexpr void (*movecon[])(void*, void*) = { &detail::s_conmove<T>... };
    static constexpr void (*copyass[])(void*, const void*) = { &detail::s_asscopy<T>... };
    static constexpr void (*moveass[])(void*, void*) = { &detail::s_assmove<T>... };
    static constexpr bool nt_copycon[] = { std::is_nothrow_copy_constructible<T>::value... };

    static constexpr bool nt_copycon_all = trait_all<std::is_nothrow_copy_constructible, T...>::value;
    static constexpr bool nt_copyass_all = trait_all<std::is_nothrow_copy_assignable, T...>::value;
    static constexpr bool nt_moveass_all = trait_all<std::is_nothrow_move_assignable, T...>::value;

  public:
    /// generic enumerate used to indicate stored type
    typedef typename generic_enum<sizeof...(T)>::type type;

    /// get the enumerate corresponding to the type specified
    template <typename S>
    static constexpr type type_enum()
    {
      return static_cast<type>(type_index<S, T...>::value);
    }

  public:
    /// default constructor, initialize with default value of first type
    variant() noexcept(std::is_nothrow_default_constructible<typename first_type<T...>::type>::value)
      : type_(static_cast<type>(0))
    {
      new (&value_) typename first_type<T...>::type();
    }

    /// construct variant directly from value
    template <typename S>
    variant(S&& value) noexcept(noexcept(S(std::forward<S>(value))))
      : type_(type_enum<typename std::decay<S>::type>())
    {
      new (&value_) typename std::decay<S>::type(std::forward<S>(value));
    }

    /// copy constructor
    variant(const variant& rhs) noexcept(nt_copycon_all)
      : type_(rhs.type_)
    {
      copycon[type_](&value_, &rhs.value_);
    }

    /// move constructor
    variant(variant&& rhs) noexcept
      : type_(rhs.type_)
    {
      movecon[type_](&value_, &rhs.value_);
    }

    /// copy assignment
    variant& operator=(const variant& rhs) noexcept(nt_copycon_all && nt_copyass_all)
    {
      if (type_ != rhs.type_)
      {
        // put nt_copycon_all here to help optimizer get rid of branch if possible
        if (nt_copycon_all || nt_copycon[type_])
        {
          // copy constructor is noexcept - safe to destroy old value first
          destroy[type_](&value_);
          copycon[rhs.type_](&value_, &rhs.value_);
          type_ = rhs.type_;
        }
        else
        {
          // copy constructor may throw - move current value out of the way before
          // destroying in case we need to rollback
          storage temp;
          movecon[type_](&temp, &value_);
          destroy[type_](&value_);
          try
          {
            copycon[rhs.type_](&value_, &rhs.value_);
            // success - destroy the temporarily moved old value and update type
            destroy[type_](&temp);
            type_ = rhs.type_;
          }
          catch (...)
          {
            // failure - rollback our temporary move and rethrow exception
            movecon[type_](&value_, &temp);
            destroy[type_](&temp);
            throw;
          }
        }
      }
      else
        copyass[type_](&value_, &rhs.value_);
      return *this;
    }

    /// move assignment
    variant& operator=(variant&& rhs) noexcept(nt_moveass_all)
    {
      if (type_ != rhs.type_)
      {
        destroy[type_](&value_);
        movecon[rhs.type_](&value_, &rhs.value_);
        type_ = rhs.type_;
      }
      else
        moveass[type_](&value_, &rhs.value_);
      return *this;
    }

    /// destructor
    ~variant() noexcept
    {
      destroy[type_](&value_);
    }

    /// return the enumerate representing the currently active type
    type which() const noexcept
    {
      return type_;
    }

    /// get a constant reference to the stored value, throw on type mismatch
    template <typename S>
    const S& get() const
    {
      if (type_ != type_enum<S>())
        throw std::runtime_error("variant type mismatch");
      return reinterpret_cast<const S&>(value_);
    }

    /// get a mutable reference to the stored value, throw on type mismatch
    template <typename S>
    S& get()
    {
      if (type_ != type_enum<S>())
        throw std::runtime_error("variant type mismatch");
      return reinterpret_cast<S&>(value_);
    }

    /// get a constant pointer to the stored value, return nullptr on mismatch
    template <typename S>
    const S* get_ptr() const noexcept
    {
      return type_ != type_enum<S>() ? nullptr : reinterpret_cast<const S*>(&value_);
    }

    /// get a mutable pointer to the stored value, return nullptr on mismatch
    template <typename S>
    S* get_ptr() noexcept
    {
      return type_ != type_enum<S>() ? nullptr : reinterpret_cast<S*>(&value_);
    }

    /// set a new value for the variant
    template <typename S>
    void set(S&& value) noexcept(   noexcept(typename std::decay<S>::type(std::forward<S>(value)))
                                 && noexcept(std::declval<typename std::add_lvalue_reference<typename std::decay<S>::type>::type>() = std::forward<S>(value)))
    {
      typedef typename std::decay<S>::type DS;
      if (type_ != type_enum<DS>())
      {
        if (noexcept(DS(std::forward<S>(value))))
        {
          // constructor is noexcept - safe to destroy old value first
          destroy[type_](&value_);
          new (&value_) DS(std::forward<S>(value));
          type_ = type_enum<DS>();
        }
        else
        {
          // constructor may throw - move current value out of the way before
          // destroying in case we need to rollback
          // note: this is unreachable code for the move case (S resolves to rvalue)
          storage temp;
          movecon[type_](&temp, &value_);
          destroy[type_](&value_);
          try
          {
            new (&value_) DS(std::forward<S>(value));
            // success - destroy the temporarily moved old value and update type
            destroy[type_](&temp);
            type_ = type_enum<DS>();
          }
          catch (...)
          {
            // failure - rollback our temporary move and rethrow exception
            movecon[type_](&value_, &temp);
            destroy[type_](&temp);
            throw;
          }
        }
      }
      else
        reinterpret_cast<DS&>(value_) = std::forward<S>(value);
    }

    /// set a new value for the variant by forwarding an unrelated type
    template <typename S, typename V>
    void set(V&& value) noexcept(   noexcept(S(std::forward<V>(value)))
                                 && noexcept(std::declval<S&>() = std::forward<V>(value)))
    {
      if (type_ != type_enum<S>())
      {
        if (noexcept(S(std::forward<V>(value))))
        {
          // constructor is noexcept - safe to destroy old value first
          destroy[type_](&value_);
          new (&value_) S(std::forward<V>(value));
          type_ = type_enum<S>();
        }
        else
        {
          // constructor may throw - move current value out of the way before
          // destroying in case we need to rollback
          storage temp;
          movecon[type_](&temp, &value_);
          destroy[type_](&value_);
          try
          {
            new (&value_) S(std::forward<V>(value));
            // success - destroy the temporarily moved old value and update type
            destroy[type_](&temp);
            type_ = type_enum<S>();
          }
          catch (...)
          {
            // failure - rollback our temporary move and rethrow exception
            movecon[type_](&value_, &temp);
            destroy[type_](&temp);
            throw;
          }
        }
      }
      else
        reinterpret_cast<S&>(value_) = std::forward<V>(value);
    }

  private:
    type    type_;
    storage value_;
  };

  template <typename... T> constexpr void (*variant<T...>::destroy[])(void*);
  template <typename... T> constexpr void (*variant<T...>::copycon[])(void*, const void*);
  template <typename... T> constexpr void (*variant<T...>::movecon[])(void*, void*);
  template <typename... T> constexpr void (*variant<T...>::copyass[])(void*, const void*);
  template <typename... T> constexpr void (*variant<T...>::moveass[])(void*, void*);
  template <typename... T> constexpr bool variant<T...>::nt_copycon[];
}

#endif
