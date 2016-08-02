// %=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%=%
// ** Ancilla Radar Quality Control System (ancilla)
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

#ifndef ANCILLA_METADATA_H
#define ANCILLA_METADATA_H

#include "angle.h"
#include "latlon.h"
#include "timestamp.h"
#include "traits.h"
#include "variant.h"
#include "xml.h"

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace rainfields {
namespace ancilla {

  class metadata
  {
  public:
    class datum;
    class meta_iterator;
    class const_meta_iterator;
    class proxy;
    class type_mismatch;

    typedef const proxy proxy_table[];
    typedef variant<
        long
      , double
      , bool
      , angle
      , timestamp
      , latlonalt
      , std::string
      , std::vector<long>
      , std::vector<double>
      > metavar;
    typedef metavar::type type;

    // types in this list are get() and set() by reference rather than value
    typedef typelist<
        latlonalt
      , std::string
      , std::vector<long>
      , std::vector<double>
      > byref_types;

  public:
    template <typename T>
    static constexpr type type_enum() { return metavar::type_enum<T>(); }

    template <class S, class T, T (S::*get_fn)() const, void (S::*set_fn)(T)>
    static constexpr proxy make_proxy(const char* name);
    template <class S, class T, class V, T (S::*get_fn)() const, void (S::*set_fn)(T)>
    static constexpr proxy make_proxy(const char* name);
    template <class S, class T, const T& (S::*get_fn)() const, void (S::*set_copy_fn)(const T&), void (S::*set_move_fn)(T&&) = nullptr>
    static constexpr proxy make_proxy(const char* name);

  public:
    metadata() = default;
    metadata(const xml::node& conf);

    metadata(const metadata& rhs) = default;
    metadata(metadata&& rhs) noexcept;
    auto operator=(const metadata& rhs) -> metadata& = default;
    auto operator=(metadata&& rhs) noexcept -> metadata&;

    virtual ~metadata() noexcept;

    auto meta_begin() -> meta_iterator;
    auto meta_begin() const -> const_meta_iterator;
    auto meta_cbegin() const -> const_meta_iterator;

    auto meta_end() -> meta_iterator;
    auto meta_end() const -> const_meta_iterator;
    auto meta_cend() const -> const_meta_iterator;

    auto meta_find(const std::string& name) -> meta_iterator;
    auto meta_find(const std::string& name) const -> const_meta_iterator;

    auto meta_insert(const std::string& name) -> meta_iterator;
    auto meta_erase(const meta_iterator& iter) -> meta_iterator;

  protected:
    virtual auto meta_proxy(size_t i) const -> const proxy*;

  private:
    typedef std::map<std::string, metavar>  extra_store;

  private:
    extra_store extra_;
  };

  class metadata::datum
  {
  public:
    auto name() const -> const char*;
    auto data_type() const -> type;
    auto bound() const -> bool;

    // if T is a member of byref_types, this function returns a const T&, otherwise T.
    template <typename T>
    auto get() const -> typename std::conditional<typelist_contains<byref_types, T>::value, const T&, T>::type;

    template <typename T>
    auto set(T&& val) -> void;

  private:
    datum(
          metadata* owner
        , size_t index
        , const proxy* prx
        , extra_store::iterator extra);

  private:
    metadata*             owner_;
    size_t                index_; // proxy index
    const proxy*          proxy_;
    extra_store::iterator extra_;

    friend class metadata;
    friend class metadata::meta_iterator;
  };

  class metadata::meta_iterator
  {
  public:
    auto operator==(const meta_iterator& rhs) -> bool;
    auto operator!=(const meta_iterator& rhs) -> bool;
    auto operator++() -> meta_iterator&;
    auto operator*() const -> datum&        { return datum_; }
    auto operator->() const -> datum*       { return &datum_; }

  private:
    meta_iterator(metadata* owner, size_t index, const proxy* prx);
    meta_iterator(metadata* owner, extra_store::iterator extra);

  private:
    mutable datum datum_;
    friend class metadata;
  };

  class metadata::const_meta_iterator
  {
  public:
    auto operator==(const const_meta_iterator& rhs) -> bool;
    auto operator!=(const const_meta_iterator& rhs) -> bool;
    auto operator++() -> const_meta_iterator&;
    auto operator*() const -> const datum&  { return datum_; }
    auto operator->() const -> const datum* { return &datum_; }

  private:
    const_meta_iterator(metadata* owner, size_t index, const proxy* prx);
    const_meta_iterator(metadata* owner, extra_store::iterator extra);

  private:
    datum   datum_;
    friend class metadata;
  };

  class metadata::type_mismatch : public std::logic_error
  {
  public:
    type_mismatch(std::string datum, type found, type requested);

    auto datum_name() const -> const std::string& { return name_; }
    auto found() const -> type           { return found_; }
    auto requested() const -> type       { return requested_; }

  private:
    std::string name_;
    type        found_;
    type        requested_;
  };

  namespace detail
  {
    // base class for a metadata proxy interface to an object value
    struct meta_proxy_base
    {
      virtual auto type() const -> metadata::type = 0;
    };

    // middle classes - proxy interface to a particular metadata type
    template <class T, bool byref = typelist_contains<metadata::byref_types, T>::value>
    struct meta_proxy_typed : meta_proxy_base
    {
      virtual auto type() const -> metadata::type { return metadata::type_enum<T>(); }
      virtual auto get(const metadata& obj) const -> T = 0;
      virtual auto set(metadata& obj, T val) const -> void = 0;
    };
    template <class T>
    struct meta_proxy_typed<T, true> : meta_proxy_base
    {
      virtual auto type() const -> metadata::type { return metadata::type_enum<T>(); }
      virtual auto get(const metadata& obj) const -> const T& = 0;
      virtual auto set(metadata& obj, const T& val) const -> void = 0;
      virtual auto set(metadata& obj, T&& val) const -> void = 0;
    };

    // leaf classes - maps concrete object get/set to the metadata type
    /* the 'byval' version gets and sets by value.  It is therefore also able to perform
     * simple conversions using static_cast.  T is the object type, V is the variant type. */
    template <class S, class T, class V, T (S::*get_fn)() const, void (S::*set_fn)(T)>
    struct meta_proxy_byval : meta_proxy_typed<V, false>
    {
      virtual auto get(const metadata& obj) const -> V 
      {
        return static_cast<V>((dynamic_cast<const S&>(obj).*get_fn)());
      }
      virtual auto set(metadata& obj, V val) const -> void 
      {
        (dynamic_cast<S&>(obj).*set_fn)(static_cast<T>(val));
      }
      static constexpr meta_proxy_byval instance = {};
    };
    template <class S, class T, class V, T (S::*get_fn)() const, void (S::*set_fn)(T)>
    constexpr meta_proxy_byval<S, T, V, get_fn, set_fn> meta_proxy_byval<S, T, V, get_fn, set_fn>::instance;

    template <
        class S
      , class T
      , const T& (S::*get_fn)() const
      , void (S::*set_copy_fn)(const T&)
      , void (S::*set_move_fn)(T&&) = nullptr>
    struct meta_proxy_byref : meta_proxy_typed<T, true>
    {
      virtual auto get(const metadata& obj) const -> const T& 
      {
        return (dynamic_cast<const S&>(obj).*get_fn)();
      }
      virtual auto set(metadata& obj, const T& val) const -> void 
      {
        (dynamic_cast<S&>(obj).*set_copy_fn)(val);
      }
      virtual auto set(metadata& obj, T&& val) const -> void 
      {
        if (set_move_fn)
          (dynamic_cast<S&>(obj).*set_move_fn)(std::move(val));
        else
          (dynamic_cast<S&>(obj).*set_copy_fn)(std::move(val));
      }
      static constexpr meta_proxy_byref instance = {};
    };
    template <class S, class T, const T& (S::*get_fn)() const, void (S::*set_copy_fn)(const T&), void (S::*set_move_fn)(T&&)>
    constexpr meta_proxy_byref<S, T, get_fn, set_copy_fn, set_move_fn> meta_proxy_byref<S, T, get_fn, set_copy_fn, set_move_fn>::instance;
  }

  struct metadata::proxy
  {
    const char* name;
    const detail::meta_proxy_base* impl;
  };

  /// create a metadata proxy manager
  template <class S, class T, T (S::*get_fn)() const, void (S::*set_fn)(T)>
  constexpr auto metadata::make_proxy(const char* name) -> proxy
  {
    return {name, &detail::meta_proxy_byval<S, T, T, get_fn, set_fn>::instance};
  }
  template <class S, class T, class V, T (S::*get_fn)() const, void (S::*set_fn)(T)>
  constexpr auto metadata::make_proxy(const char* name) -> proxy
  {
    return {name, &detail::meta_proxy_byval<S, T, V, get_fn, set_fn>::instance};
  }
  template <class S, class T, const T& (S::*get_fn)() const, void (S::*set_copy_fn)(const T&), void (S::*set_move_fn)(T&&)>
  constexpr auto metadata::make_proxy(const char* name) -> proxy
  {
    return {name, &detail::meta_proxy_byref<S, T, get_fn, set_copy_fn, set_move_fn>::instance};
  }

  template <typename T>
  auto metadata::datum::get() const -> typename std::conditional<typelist_contains<byref_types, T>::value, const T&, T>::type
  {
    if (proxy_)
    {
      if (auto ptr = dynamic_cast<const detail::meta_proxy_typed<T>*>(proxy_->impl))
        return ptr->get(*owner_);
      throw type_mismatch(proxy_->name, proxy_->impl->type(), type_enum<T>());
    }
    else
    {
      if (extra_->second.which() == type_enum<T>())
        return extra_->second.get<T>();
      throw type_mismatch(extra_->first, extra_->second.which(), type_enum<T>());
    }
  }

  template <typename T>
  auto metadata::datum::set(T&& val) -> void
  {
    typedef typename std::decay<T>::type DT;
    if (proxy_)
    {
      if (auto ptr = dynamic_cast<const detail::meta_proxy_typed<DT>*>(proxy_->impl))
        return ptr->set(*owner_, std::forward<T>(val));
      throw type_mismatch(proxy_->name, proxy_->impl->type(), type_enum<DT>());
    }
    else
      extra_->second.set(std::forward<T>(val));
  }
}}

#endif
