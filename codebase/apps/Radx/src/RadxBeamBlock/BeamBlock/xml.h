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

#ifndef RAINUTIL_XML_H
#define RAINUTIL_XML_H

#include "string_utils.h"

#include <istream>
#include <iterator>
#include <exception>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace rainfields {
namespace xml {

  class node;
  class attribute;

  typedef std::unordered_set<std::string> string_store;

  class parse_error : public std::exception
  {
  public:
    virtual auto what() const noexcept -> const char*;

    auto description() const -> const char*         { return desc_; }
    auto line() const -> int                        { return line_; }
    auto col() const -> int                         { return col_; }

  private:
    parse_error(const char* desc, const char* where);

    auto determine_line_col(const char* src) -> void;

    const char*   desc_;
    const char*   where_;
    int           line_;
    int           col_;
    mutable char  what_[128];

    friend class attribute;
    friend class node;
    friend class document;
    friend auto skip_pi(const char*& src) -> void;
    friend auto skip_comment(const char*& src) -> void;
    friend auto skip_doctype(const char*& src) -> void;
    friend auto resolve_entities(std::string& src, const char* where_end) -> void;
  };

  class path_error : public std::exception
  {
  public:
    path_error(const char* name, bool is_attribute);

    virtual auto what() const noexcept -> const char*;

    auto name() const -> const std::string&         { return name_; }
    auto is_attribute() const -> bool               { return is_attribute_; }

  private:
    std::string         name_;
    bool                is_attribute_;
    mutable std::string description_;
  };

  class attribute_store : private std::vector<attribute>
  {
  private:
    typedef std::vector<attribute> base;

  public:
    typedef base::iterator iterator;
    typedef base::const_iterator const_iterator;
    typedef base::reverse_iterator reverse_iterator;
    typedef base::const_reverse_iterator const_reverse_iterator;

  public:
    attribute_store(const attribute_store& rhs) = delete;
    attribute_store(attribute_store&& rhs) noexcept;

    auto operator=(const attribute_store& rhs) -> attribute_store& = delete;
    auto operator=(attribute_store&& rhs) noexcept -> attribute_store&;

    ~attribute_store() noexcept = default;

    using base::begin;
    using base::end;
    using base::rbegin;
    using base::rend;
    using base::cbegin;
    using base::cend;
    using base::crbegin;
    using base::crend;

    using base::size;
    using base::empty;
    using base::reserve;

    using base::operator[];
    auto operator[](const char* name) -> attribute&;
    auto operator[](const char* name) const -> const attribute&;
    auto operator[](const std::string& name) -> attribute&;
    auto operator[](const std::string& name) const -> const attribute&;

    auto find(const char* name) -> iterator;
    auto find(const char* name) const -> const_iterator;
    auto find(const std::string& name) -> iterator;
    auto find(const std::string& name) const -> const_iterator;

    auto insert(const std::string& name) -> iterator;
    auto insert(std::string&& name) -> iterator;

    auto find_or_insert(const char* name) -> iterator;
    auto find_or_insert(const std::string& name) -> iterator;

    using base::erase;
    using base::clear;

  private:
    attribute_store(string_store* strings) : strings_(strings) { }

  private:
    string_store* strings_;

    friend class node;
    friend class node_store;
  };

  class node_store : private std::vector<node>
  {
  private:
    typedef std::vector<node> base;

  public:
    node_store(const node_store& rhs) = delete;
    node_store(node_store&& rhs) noexcept;

    auto operator=(const node_store& rhs) -> node_store& = delete;
    auto operator=(node_store&& rhs) noexcept -> node_store&;

    ~node_store() noexcept = default;

    using base::begin;
    using base::end;
    using base::rbegin;
    using base::rend;
    using base::cbegin;
    using base::cend;
    using base::crbegin;
    using base::crend;

    using base::size;
    using base::empty;
    using base::reserve;

    using base::operator[];
    using base::front;
    using base::back;

    using base::clear;

    auto operator[](const char* name) -> node&;
    auto operator[](const char* name) const -> const node&;
    auto operator[](const std::string& name) -> node&;
    auto operator[](const std::string& name) const -> const node&;

    auto find(const char* name) -> iterator;
    auto find(const char* name) const -> const_iterator;
    auto find(const std::string& name) -> iterator;
    auto find(const std::string& name) const -> const_iterator;

    auto push_back(const std::string& name) -> iterator;
    auto push_back(std::string&& name) -> iterator;
    using base::pop_back;

    auto insert(iterator position, const std::string& name) -> iterator;
    auto insert(iterator position, std::string&& name) -> iterator;
    using base::erase;

    auto find_or_insert(const char* name) -> iterator;
    auto find_or_insert(const std::string& name) -> iterator;

  private:
    node_store(node* owner) : owner_(owner) { }

  private:
    node* owner_;

    friend class node;
  };

  class attribute
  {
  public:
    attribute(const attribute& rhs) = delete;
    attribute(attribute&& rhs) noexcept = default;

    auto operator=(const attribute& rhs) -> attribute& = delete;
    auto operator=(attribute&& rhs) noexcept -> attribute& = default;

    ~attribute() noexcept = default;

    auto name() const -> const std::string&           { return *name_; }
    auto set_name(const std::string& val) -> void;

    auto get() const -> const std::string&            { return *value_; }
    auto set(const std::string& val) -> void;

    // allow conversion to all the primitive types (including traited enums)
    operator bool() const                             { return from_string<bool>(*value_); }
    operator char() const                             { return from_string<char>(*value_); }
    operator signed char() const                      { return from_string<signed char>(*value_); }
    operator unsigned char() const                    { return from_string<unsigned char>(*value_); }
    operator short() const                            { return from_string<short>(*value_); }
    operator unsigned short() const                   { return from_string<unsigned short>(*value_); }
    operator int() const                              { return from_string<int>(*value_); }
    operator unsigned int() const                     { return from_string<unsigned int>(*value_); }
    operator long() const                             { return from_string<long>(*value_); }
    operator unsigned long() const                    { return from_string<unsigned long>(*value_); }
    operator long long() const                        { return from_string<long long>(*value_); }
    operator unsigned long long() const               { return from_string<unsigned long long>(*value_); }
    operator float() const                            { return from_string<float>(*value_); }
    operator double() const                           { return from_string<double>(*value_); }
    operator long double() const                      { return from_string<long double>(*value_); }
    operator const std::string&() const               { return *value_; }
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type >
    operator T() const
    {
      return from_string<T>(*value_);
    }

    auto set(bool val) -> void                        { set(to_string(val)); }
    auto set(char val) -> void                        { set(to_string(val)); }
    auto set(signed char val) -> void                 { set(to_string(val)); }
    auto set(unsigned char val) -> void               { set(to_string(val)); }
    auto set(short val) -> void                       { set(to_string(val)); }
    auto set(unsigned short val) -> void              { set(to_string(val)); }
    auto set(int val) -> void                         { set(to_string(val)); }
    auto set(unsigned int val) -> void                { set(to_string(val)); }
    auto set(long val) -> void                        { set(to_string(val)); }
    auto set(unsigned long val) -> void               { set(to_string(val)); }
    auto set(long long val) -> void                   { set(to_string(val)); }
    auto set(unsigned long long val) -> void          { set(to_string(val)); }
    auto set(float val) -> void                       { set(to_string(val)); }
    auto set(double val) -> void                      { set(to_string(val)); }
    auto set(long double val) -> void                 { set(to_string(val)); }
    auto set(const char* val) -> void                 { set(std::string(val)); }
    template <typename T>
    auto set(T val) -> typename std::enable_if<std::is_enum<T>::value, void>::type
    {
      set(to_string(val));
    }

  private:
    // this allows us to use a constructor in a container's emplace methods while 
    // still ensuring that it uncallable (private) from the user's point of view
    struct private_constructor { };

  public:
    attribute(
          string_store* strings
        , const char*& src
        , std::string& tmp
        , private_constructor);
    attribute(
          string_store* strings
        , const attribute& rhs
        , private_constructor);
    attribute(
          string_store* strings
        , const std::string* name
        , private_constructor);

    auto write(std::ostream& out) const -> void;

  private:
    string_store*       strings_;
    const std::string*  name_;
    const std::string*  value_;

    friend class node;
    friend class attribute_store;
  };

  /* mixed mode elements are emulated using elements with an empty name */
  class node
  {
  public:
    enum class type
    {
        element_simple      //!< empty, or simple text contents element
      , element_compound    //!< element containing children (elements and/or text)
      , text_block          //!< text only block (child of a mixed element)
    };

  public:
    node(const node& rhs) = delete;
    node(node&& rhs) noexcept;

    auto operator=(const node& rhs) -> node& = delete;
    auto operator=(node&& rhs) noexcept -> node&;

    ~node() noexcept = default;

    auto write(std::ostream& out) const -> void;

    auto node_type() const -> enum type;

    auto name() const -> const std::string&                             { return *name_; }
    auto set_name(const std::string& val) -> void;

    // attribute access: only valid for 'element_simple' and 'element_compund' nodes
    auto attributes() -> attribute_store&                               { return attributes_; }
    auto attributes() const -> const attribute_store&                   { return attributes_; }
    auto operator()(const char* name) -> attribute&                     { return attributes_[name]; }
    auto operator()(const char* name) const -> const attribute&         { return attributes_[name]; }
    auto operator()(const std::string& name) -> attribute&              { return attributes_[name]; }
    auto operator()(const std::string& name) const -> const attribute&  { return attributes_[name]; }
    // optional attribute access:
    auto operator()(const char* name, bool def) const -> bool;
    auto operator()(const char* name, char def) const -> char;
    auto operator()(const char* name, signed char def) const -> signed char;
    auto operator()(const char* name, unsigned char def) const -> unsigned char;
    auto operator()(const char* name, short def) const -> short;
    auto operator()(const char* name, unsigned short def) const -> unsigned short;
    auto operator()(const char* name, int def) const -> int;
    auto operator()(const char* name, unsigned int def) const -> unsigned int;
    auto operator()(const char* name, long def) const -> long;
    auto operator()(const char* name, unsigned long def) const -> unsigned long;
    auto operator()(const char* name, long long def) const -> long long;
    auto operator()(const char* name, unsigned long long def) const -> unsigned long long;
    auto operator()(const char* name, float def) const -> float;
    auto operator()(const char* name, double def) const -> double;
    auto operator()(const char* name, long double def) const -> long double;
    auto operator()(const char* name, const char* def) const -> const char*;
    template <typename T>
    auto operator()(const char* name, T def) const -> typename std::enable_if<std::is_enum<T>::value, T>::type;

    // child access: only valid for 'element_compound' nodes
    auto children() -> node_store&                                      { return children_; }
    auto children() const -> const node_store&                          { return children_; }
    auto operator[](const char* name) -> node&                          { return children_[name]; }
    auto operator[](const char* name) const -> const node&              { return children_[name]; }
    auto operator[](const std::string& name) -> node&                   { return children_[name]; }
    auto operator[](const std::string& name) const -> const node&       { return children_[name]; }

    auto get() const -> const std::string&                              { return *value_; }
    auto set(const std::string& val) -> void;

    operator bool() const                                               { return from_string<bool>(*value_); }
    operator char() const                                               { return from_string<char>(*value_); }
    operator signed char() const                                        { return from_string<signed char>(*value_); }
    operator unsigned char() const                                      { return from_string<unsigned char>(*value_); }
    operator short() const                                              { return from_string<short>(*value_); }
    operator unsigned short() const                                     { return from_string<unsigned short>(*value_); }
    operator int() const                                                { return from_string<int>(*value_); }
    operator unsigned int() const                                       { return from_string<unsigned int>(*value_); }
    operator long() const                                               { return from_string<long>(*value_); }
    operator unsigned long() const                                      { return from_string<unsigned long>(*value_); }
    operator long long() const                                          { return from_string<long long>(*value_); }
    operator unsigned long long() const                                 { return from_string<unsigned long long>(*value_); }
    operator float() const                                              { return from_string<float>(*value_); }
    operator double() const                                             { return from_string<double>(*value_); }
    operator long double() const                                        { return from_string<long double>(*value_); }
    operator const std::string&() const                                 { return *value_; }
    template <typename T, typename = typename std::enable_if<std::is_enum<T>::value, T>::type >
    operator T() const
    {
      return from_string<T>(*value_);
    }

    auto set(bool val) -> void                                          { set(to_string(val)); }
    auto set(char val) -> void                                          { set(to_string(val)); }
    auto set(signed char val) -> void                                   { set(to_string(val)); }
    auto set(unsigned char val) -> void                                 { set(to_string(val)); }
    auto set(short val) -> void                                         { set(to_string(val)); }
    auto set(unsigned short val) -> void                                { set(to_string(val)); }
    auto set(int val) -> void                                           { set(to_string(val)); }
    auto set(unsigned int val) -> void                                  { set(to_string(val)); }
    auto set(long val) -> void                                          { set(to_string(val)); }
    auto set(unsigned long val) -> void                                 { set(to_string(val)); }
    auto set(long long val) -> void                                     { set(to_string(val)); }
    auto set(unsigned long long val) -> void                            { set(to_string(val)); }
    auto set(float val) -> void                                         { set(to_string(val)); }
    auto set(double val) -> void                                        { set(to_string(val)); }
    auto set(long double val) -> void                                   { set(to_string(val)); }
    auto set(const char* val) -> void                                   { set(std::string(val)); }
    template <typename T>
    auto set(T val) -> typename std::enable_if<std::is_enum<T>::value, void>::type
    {
      set(to_string(val));
    }

  private:
    // this allows us to use a constructor in a container's emplace methods while 
    // still ensuring that it uncallable (private) from the user's point of view
    struct private_constructor { };

  public:
    node(private_constructor);
    node(
          string_store* strings
        , const std::string* value
        , private_constructor);
    node(
          string_store* strings
        , const char*& src
        , std::string& tmp
        , private_constructor);
    node(
          string_store* strings
        , const node& rhs
        , private_constructor);
    node(
          string_store* strings
        , const std::string* name
        , bool // dummy flag to distinguish constructor
        , private_constructor);

  private:
    attribute_store     attributes_;  // contains a pointer to the strings store
    const std::string*  name_;
    const std::string*  value_;       // only meaningful if a text node
    node_store          children_;

    friend class document;
    friend class node_store;
  };

  template <typename T>
  auto node::operator()(const char* name, T def) const -> typename std::enable_if<std::is_enum<T>::value, T>::type
  {
    auto i = attributes_.find(name);
    return i != attributes_.end() ? static_cast<T>(*i) : def;
  }

  class document
  {
  public:
    /// create an empty document with an empty root node
    document();
    /// parse a document from an input stream
    document(std::istream& source);
    /// parse a document from an input stream
    document(std::istream&& source);
    /// parse a document from a string
    document(const char* source);
    /// create a document by copying a node from another document as the root
    document(const xml::node& root);

    document(const document& rhs);
    document(document&& rhs) noexcept;

    auto operator=(const document& rhs) -> document&;
    auto operator=(document&& rhs) noexcept -> document&;

    ~document() noexcept = default;

    auto write(std::ostream& out) -> void;
    auto write(std::ostream&& out) -> void;

    auto root() -> node&             { return root_; }
    auto root() const -> const node& { return root_; }

  private:
    string_store  strings_;
    node          root_;
  };
}}

#endif
