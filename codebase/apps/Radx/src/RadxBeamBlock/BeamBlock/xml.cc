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

#include "xml.h"

#include <cstdio>

using namespace rainfields::xml;

#if 0
static_assert(
      std::is_nothrow_move_constructible<attribute>::value
    , "attribute not move constructible");
static_assert(
      std::is_nothrow_move_assignable<attribute>::value
    , "attribute not move assignable");
static_assert(
      std::is_nothrow_move_constructible<node>::value
    , "node not move constructible");
static_assert(
      std::is_nothrow_move_assignable<node>::value
    , "node not move assignable");
#endif

namespace rainfields {
namespace xml {

  // this is thread safe because we only ever use it in comparisons
  static const std::string empty_string;

  auto skip_pi(const char*& src) -> void
  {
    // loop until we find a '?>'
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (c != '?')
        continue;

      c = *++src;
      if (c != '>')
        continue;

      ++src;
      return;
    }

    throw parse_error("unterminated processing instruction '<?'", src);
  }

  auto skip_comment(const char*& src) -> void
  {
    // check that we have the '--'
    if (*++src != '-')
      throw parse_error("invalid comment", src);
    if (*++src != '-')
      throw parse_error("invalid comment", src);

    // loop until we find a '--'
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (c != '-')
        continue;

      c = *++src;
      if (c != '-')
        continue;

      c = *++src;
      if (c != '>')
        throw parse_error("comment body contains '--'", src);

      ++src;
      return;
    }

    throw parse_error("unterminated comment", src);
  }

  /* this function is currently only able to skip simple DOCTYPE declarations.
   * if you need to skip declarations that contain internal subsets you are in
   * for some fun... */
  auto skip_doctype(const char*& src) -> void
  {
    ++src;
    if (   src[0] != 'D' || src[1] != 'O' || src[2] != 'C'
        || src[3] != 'T' || src[4] != 'Y' || src[5] != 'P' || src[6] != 'E')
      throw parse_error("invalid DOCTYPE", src);
    src += 7;

    char quote_char = '\0';
    for (char c = *++src; c != '\0'; c = *++src)
    {
      if (quote_char == '\0')
      {
        if (c == '>')
        {
          ++src;
          return;
        }
        if (c == '"' || c == '\'')
          quote_char = c;
      }
      else if (c == quote_char)
        quote_char = '\0';
    }

    throw parse_error("unterminated DOCTYPE", src);
  }

  /* where_end should point to the character after 'src' in the stream. it 
   * is only used to provide location details when an exception is thrown */
  auto resolve_entities(std::string& src, const char* where_end) -> void
  {
    // search backward through the string for an '&'
    for (auto i = src.rbegin(); i != src.rend(); ++i)
    {
      if (*i != '&')
        continue;
      
      // got an entity, now loop forward until the ';'
      std::string::iterator s = i.base() - 1;
      std::string::iterator e = s;
      while (e != src.end() && *e != ';')
        ++e;
      if (e == src.end())
        throw parse_error(
              "unterminated character entity"
            , where_end - (i - src.rbegin() + 1));

      // determine which entity we have found.  if we wanted to support 
      // custom entities, they would need to be detected here
      auto el = e - s;
      if (el == 3 && *(s + 1) == 'l' && *(s + 2) == 't')
        *e = '<';
      else if (el == 3 && *(s + 1) == 'g' && *(s + 2) == 't')
        *e = '>';
      else if (el == 4 && *(s + 1) == 'a' && *(s + 2) == 'm' && *(s + 3) == 'p')
        *e = '&';
      else if (el == 5 && *(s + 1) == 'a' && *(s + 2) == 'p' && *(s + 3) == 'o' && *(s + 4) == 's')
        *e = '\'';
      else if (el == 5 && *(s + 1) == 'q' && *(s + 2) == 'u' && *(s + 3) == 'o' && *(s + 4) == 't')
        *e = '"';
      else
        throw parse_error(
              "unterminated character entity"
            , where_end - (i - src.rbegin() + 1));

      // erase the entity
      src.erase((i.base() - 1), e);
    }
  }
}}

parse_error::parse_error(const char* desc, const char* where)
  : desc_(desc)
  , where_(where)
  , line_(0)
  , col_(0)
{

}

auto parse_error::what() const noexcept -> const char*
{
  snprintf(
        what_
      , std::extent<decltype(what_)>::value
      , "xml parse error: %s at line %d col %d"
      , desc_
      , line_
      , col_);
  return what_;
}

auto parse_error::determine_line_col(const char* src) -> void
{
  for (line_ = col_ = 1; src < where_; ++src)
  {
    if (*src == '\n')
    {
      ++line_;
      col_ = 1;
    }
    else
      ++col_;
  }
}

path_error::path_error(const char* name, bool is_attribute)
  : name_(name)
  , is_attribute_(is_attribute)
{

}

auto path_error::what() const noexcept -> const char*
{
  description_.assign("xml path error: requested child ");
  if (is_attribute_)
    description_.append("attribute '");
  else
    description_.append("node '");
  description_.append(name_);
  description_.append("' does not exist");
  return description_.c_str();
}

attribute_store::attribute_store(attribute_store&& rhs) noexcept
  : base(std::move(rhs))
  , strings_(rhs.strings_)
{

}

auto attribute_store::operator=(attribute_store&& rhs) noexcept -> attribute_store&
{
  base::operator=(std::move(rhs));
  strings_ = rhs.strings_;
  return *this;
}

auto attribute_store::operator[](const char* name) -> attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, true);
}

auto attribute_store::operator[](const char* name) const -> const attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, true);
}

auto attribute_store::operator[](const std::string& name) -> attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), true);
}

auto attribute_store::operator[](const std::string& name) const -> const attribute&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), true);
}

auto attribute_store::find(const char* name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const char* name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const std::string& name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::find(const std::string& name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto attribute_store::insert(const std::string& name) -> iterator
{
  emplace_back(
        strings_
      , &*strings_->insert(name).first
      , attribute::private_constructor());
  return end() - 1;
}

auto attribute_store::insert(std::string&& name) -> iterator
{
  emplace_back(
        strings_
      , &*strings_->insert(std::move(name)).first
      , attribute::private_constructor());
  return end() - 1;
}

auto attribute_store::find_or_insert(const char* name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(name);
}

auto attribute_store::find_or_insert(const std::string& name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(name);
}

node_store::node_store(node_store&& rhs) noexcept
  : base(std::move(rhs))
  , owner_(rhs.owner_)
{

}

auto node_store::operator=(node_store&& rhs) noexcept -> node_store&
{
  base::operator=(std::move(rhs));
  owner_ = rhs.owner_;
  return *this;
}

auto node_store::operator[](const char* name) -> node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, false);
}

auto node_store::operator[](const char* name) const -> const node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name, false);
}

auto node_store::operator[](const std::string& name) -> node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), false);
}

auto node_store::operator[](const std::string& name) const -> const node&
{
  for (auto& i : *this)
    if (i.name() == name)
      return i;
  throw path_error(name.c_str(), false);
}

auto node_store::find(const char* name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const char* name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const std::string& name) -> iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::find(const std::string& name) const -> const_iterator
{
  auto i = begin();
  while (i != end() && i->name() != name)
    ++i;
  return i;
}

auto node_store::push_back(const std::string& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  emplace_back(
        owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(name).first
      , true
      , node::private_constructor());
  return end() - 1;
}

auto node_store::push_back(std::string&& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  emplace_back(
        owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(std::move(name)).first
      , true
      , node::private_constructor());
  return end() - 1;
}

auto node_store::insert(iterator position, const std::string& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  return emplace(
        position
      , owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(name).first
      , true
      , node::private_constructor());
}

auto node_store::insert(iterator position, std::string&& name) -> iterator
{
  if (!owner_->value_->empty())
    throw std::logic_error("xml logic error: adding child node to non-empty element_simple");
  return emplace(
        position
      , owner_->attributes_.strings_
      , &*owner_->attributes_.strings_->insert(std::move(name)).first
      , true
      , node::private_constructor());
}

auto node_store::find_or_insert(const char* name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(end(), name);
}

auto node_store::find_or_insert(const std::string& name) -> iterator
{
  auto i = find(name);
  return i != end() ? i : insert(end(), name);
}

// this constructor is used to parse an attribute from a string
attribute::attribute(
      string_store* strings
    , const char*& src
    , std::string& tmp
    , private_constructor)
  : strings_(strings)
{
  size_t len;
  char c = *src;

  // extract the name
  len = 0;
  while (c != '\0' && c != '=' && c != ' ' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("attribute missing name", src);
  tmp.assign(src - len, len);
  name_ = &*strings_->insert(tmp).first;

  // extract the =
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '=')
    throw parse_error("attribute missing '='", src);
  c = *++src;

  // extract the leading "
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '"')
    throw parse_error("attribute missing leading '\"'", src);
  c = *++src;

  // extract the value
  len = 0;
  while (c != '\0' && c != '"')
    c = *++src, ++len;
  tmp.assign(src - len, len);
  resolve_entities(tmp, src);
  value_ = &*strings_->insert(tmp).first;

  // extract the trailing "
  if (c != '"')
    throw parse_error("attribute missing '\"'", src);
  ++src;
}

// this constructor is used to copy an attribute between documents
attribute::attribute(string_store* strings, const attribute& rhs, private_constructor)
  : strings_(strings)
  , name_(&*strings_->insert(*rhs.name_).first)
  , value_(&*strings_->insert(*rhs.value_).first)
{

}

// this constructor is used by attribute_store when appending a new attribute
attribute::attribute(string_store* strings, const std::string* name, private_constructor)
  : strings_(strings)
  , name_(name)
  , value_(&empty_string)
{

}

auto attribute::write(std::ostream& out) const -> void
{
  out << *name_ << "=\"";
  for (auto c : *value_)
    if (c == '<')
      out << "&lt;";
    else if (c == '&')
      out << "&amp;";
    else if (c == '"')
      out << "&quot;";
    else
      out << c;
  out << "\" ";
}

auto attribute::set_name(const std::string& val) -> void
{
  name_ = &*strings_->insert(val).first;
}

auto attribute::set(const std::string& val) -> void
{
  value_ = &*strings_->insert(val).first;
}

// this constructor is used to initialize a text_block
node::node(
      string_store* strings
    , const std::string* value
    , private_constructor)
  : attributes_(strings)
  , name_(&empty_string)
  , value_(value)
  , children_(this)
{ }

// this constructor is used to parse an element from a string
node::node(
      string_store* strings
    , const char*& src
    , std::string& tmp
    , private_constructor)
  : attributes_(strings)
  , value_(&empty_string)
  , children_(this)
{
  size_t len;
  char c = *src;

  // extract the name
  len = 0;
  while (c != '\0' && c != ' ' && c != '>' && c != '/' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("element missing name", src);
  tmp.assign(src - len, len);
  name_ = &*attributes_.strings_->insert(tmp).first;

  // extract any attributes
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
      c = *++src;
    else if (c == '/' || c == '>')
      break;
    else
    {
      attributes_.emplace_back(attributes_.strings_, src, tmp, attribute::private_constructor());
      c = *src;
    }
  }

  // is it an empty element?
  if (c == '/')
  {
    c = *++src;
    if (c != '>')
      throw parse_error("invalid empty element tag", src);
    ++src;
    return;
  }
  if (c != '>')
    throw parse_error("invalid start element tag", src);
  c = *++src;

  // extract children
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
      c = *++src;
    else if (c == '<')
    {
      // end-tag, comment, processing instruction or child element?
      c = *++src;
      if (c == '/')
        break;
      if (c == '\0')
        throw parse_error("unexpected end of file", src);
      if (c == ' ' || c == '\t' || c == '\n')
        throw parse_error("unexpected whitespace in tag", src);
      if (c == '!')
        /* Note: CDATA will also take this path (but cause a parse error) */
        skip_comment(src);
      else if (c == '?')
        skip_pi(src);
      else
      {
        // if we get an element subsequent to initialzing as a text only element
        // then upgrade us to a mixed content element first
        if (value_ != &empty_string)
        {
          children_.emplace_back(attributes_.strings_, value_, private_constructor());
          value_ = &empty_string;
        }
        children_.emplace_back(attributes_.strings_, src, tmp, private_constructor());
      }
      c = *src;
    }
    else
    {
      // child text

      // if we are already a text node, we must have encountered a comment or 
      // processing instruction after text. upgrade us to a mixed content element
      if (value_ != &empty_string)
        children_.emplace_back(attributes_.strings_, value_, private_constructor());

      // extract until end of the text
      len = 0;
      while (c != '\0' && c != '<')
        c = *++src, ++len;

      // strip trailing whitespace
      size_t strip = 0;
      for (; strip < len; ++strip)
      {
        char cc = *(src - strip - 1);
        if (cc != ' ' && cc != '\t' && cc != '\n')
          break;
      }
      tmp.assign(src - len, len - strip);
      resolve_entities(tmp, src);
      value_ = &*attributes_.strings_->insert(tmp).first;

      // is this a mixed content element?
      if (!children_.empty())
      {
        children_.emplace_back(attributes_.strings_, value_, private_constructor());
        value_ = &empty_string;
      }
    }
  }
  if (c == '\0')
    throw parse_error("unexpected end of file", src);
  c = *++src;

  // extract end tag
  len = 0;
  while (c != '\0' && c != '>' && c != ' ' && c != '\t' && c != '\n')
    c = *++src, ++len;
  if (len == 0)
    throw parse_error("invalid element end tag", src);
  tmp.assign(src - len, len);
  if (tmp != *name_)
    throw parse_error("element end tag mismatch", src);

  // extract the trailing >
  while (c != '\0' && (c == ' ' || c == '\t' || c == '\n'))
    c = *++src;
  if (c != '>')
    throw parse_error("invalid element end tag", src);
  ++src;
}

// this constructor is used when copying a node to a new document
node::node(string_store* strings, const node& rhs, private_constructor)
  : attributes_(strings)
  , name_(&*attributes_.strings_->insert(*rhs.name_).first)
  , value_(&*attributes_.strings_->insert(*rhs.value_).first)
  , children_(this)
{
  attributes_.reserve(rhs.attributes_.size());
  for (auto& i : rhs.attributes_)
    attributes_.emplace_back(strings, i, attribute::private_constructor());

  children_.reserve(rhs.children_.size());
  for (auto& i : rhs.children_)
    children_.emplace_back(strings, i, private_constructor());
}

// this constructor is used to insert a new node
node::node(
      string_store* strings
    , const std::string* name
    , bool
    , private_constructor)
  : attributes_(strings)
  , name_(name)
  , value_(&empty_string)
  , children_(this)
{ }

node::node(node&& rhs) noexcept
  : attributes_(std::move(rhs.attributes_))
  , name_(rhs.name_)
  , value_(rhs.value_)
  , children_(std::move(rhs.children_))
{
  children_.owner_ = this;
}

auto node::operator=(node&& rhs) noexcept -> node&
{
  attributes_ = std::move(rhs.attributes_);
  name_ = rhs.name_;
  value_ = rhs.value_;
  children_ = std::move(rhs.children_);
  children_.owner_ = this;
  return *this;
}

auto node::write(std::ostream& out) const -> void
{
  if (!children_.empty())
  {
    out << "<" << *name_ << " ";
    for (auto& i : attributes_)
      i.write(out);
    out << ">";
    for (auto& i : children_)
      i.write(out);
    out << "</" << *name_ << ">";
  }
  else if (!name_->empty())
  {
    out << "<" << *name_ << " ";
    for (auto& i : attributes_)
      i.write(out);
    if (value_->empty())
      out << "/>";
    else
    {
      out << ">";
      for (auto c : *value_)
        if (c == '<')
          out << "&lt;";
        else if (c == '>')
          out << "&gt;";
        else if (c == '&')
          out << "&amp;";
        else
          out << c;
      out << "</" << *name_ << ">";
    }
  }
  else
  {
    for (auto c : *value_)
      if (c == '<')
        out << "&lt;";
      else if (c == '>')
        out << "&gt;";
      else if (c == '&')
        out << "&amp;";
      else
        out << c;
  }
}

auto node::node_type() const -> enum type
{
  if (!children_.empty())
    return type::element_compound;
  if (!name_->empty())
    return type::element_simple;
  return type::text_block;
}

auto node::set_name(const std::string& val) -> void
{
  name_ = &*attributes_.strings_->insert(val).first;
}

auto node::operator()(const char* name, bool def) const -> bool
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, char def) const -> char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, signed char def) const -> signed char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned char def) const -> unsigned char
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, short def) const -> short
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned short def) const -> unsigned short
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, int def) const -> int
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned int def) const -> unsigned int
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long def) const -> long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned long def) const -> unsigned long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long long def) const -> long long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, unsigned long long def) const -> unsigned long long
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, float def) const -> float
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, double def) const -> double
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, long double def) const -> long double
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? *i : def;
}

auto node::operator()(const char* name, const char* def) const -> const char*
{
  auto i = attributes_.find(name);
  return i != attributes_.end() ? i->get().c_str() : def;
}

auto node::set(const std::string& val) -> void
{
  if (!children_.empty())
    throw std::logic_error("xml logic error: attempt to set simple contents in element_compound");
  value_ = &*attributes_.strings_->insert(val).first;
}

document::document()
  : root_(&strings_, &*strings_.insert("root").first, true, node::private_constructor())
{

}

document::document(std::istream& source)
  : document(std::string(
            std::istreambuf_iterator<char>(source)
          , std::istreambuf_iterator<char>()).c_str())
{

}

document::document(std::istream&& source)
  : document(std::string(
            std::istreambuf_iterator<char>(source)
          , std::istreambuf_iterator<char>()).c_str())
{

}

document::document(const char* source) try
  : root_(&strings_, &empty_string, node::private_constructor())
{
  const char* src = source;
  std::string tmp;
  char c = *src;

#if 0 // enable to force checking of <?xml ... ?> at start of document
  // skip the xml declaration
  if (src[0] != '<' || src[1] != '?' || src[2] != 'x' || src[3] != 'm' || src[4] != 'l')
    throw parse_error("missing xml declaration", src);
  src += 5;
  skip_pi(src);
#endif

  // skip comments, processing instructions and the document type declaration
  while (c != '\0')
  {
    if (c == ' ' || c == '\t' || c == '\n')
    {
      c = *++src;
      continue;
    }

    if (c != '<')
      throw parse_error("unexpected character", src);

    c = *++src;
    if (c == '?')
      skip_pi(src);
    else if (c == '!' && src[1] == '-')
      skip_comment(src);
    else if (c == '!')
      skip_doctype(src);
    else
    {
      root_ = node(&strings_, src, tmp, node::private_constructor());
      return;
    }
    c = *src;
  }

  throw parse_error("missing root element", src);
}
catch (parse_error& err)
{
  err.determine_line_col(source);
  throw;
}

document::document(const node& root)
  : root_(&strings_, root, node::private_constructor())
{
  
}

document::document(const document& rhs)
  : strings_(rhs.strings_)
  , root_(&strings_, rhs.root_, node::private_constructor())
{

}

document::document(document&& rhs) noexcept
  : root_(std::move(rhs.root_))
{
  // only a.swap(b) is guarenteed to _not_ perform any copy/move/assignment on the
  // items held by a container. this ensures that the string pointers held by the
  // nodes and attributes shall not be invalidated (c++ 23.2.1 item 8)
  strings_.swap(rhs.strings_);
}

auto document::operator=(document&& rhs) noexcept -> document&
{
  strings_.swap(rhs.strings_);
  root_ = std::move(rhs.root_);
  return *this;
}

auto document::write(std::ostream& out) -> void
{
  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
  root_.write(out);
}

auto document::write(std::ostream&& out) -> void
{
  write(out);
}

