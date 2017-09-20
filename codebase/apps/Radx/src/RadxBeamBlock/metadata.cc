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

#include "metadata.h"

#include "string_utils.h"

using namespace rainfields::ancilla;

metadata::metadata(const xml::node& conf)
{
  for (auto& x : conf.children())
  {
    const std::string& name = x("name");
    const std::string& type = x("type");
    if (type == "integer")
      extra_.insert(std::make_pair(name, metavar{static_cast<long>(x)}));
    else if (type == "real")
      extra_.insert(std::make_pair(name, metavar{static_cast<double>(x)}));
    else if (type == "boolean")
      extra_.insert(std::make_pair(name, metavar{static_cast<bool>(x)}));
    else if (type == "angle")
      extra_.insert(std::make_pair(name, metavar{from_string<angle>(x.get())}));
    else if (type == "timestamp")
      extra_.insert(std::make_pair(name, metavar{from_string<timestamp>(x.get())}));
    else if (type == "latlonalt")
      extra_.insert(std::make_pair(name, metavar{from_string<latlonalt>(x.get())}));
    else if (type == "string")
      extra_.insert(std::make_pair(name, metavar{x.get()}));
    else if (type == "integer_array")
      throw std::logic_error{"metadata: unimplemented feature"};
    else if (type == "real_array")
      throw std::logic_error{"metadata: unimplemented feature"};
    else
      throw std::runtime_error{msg{} << "invalid metadata type: " << type};
  }
}

metadata::metadata(metadata&& rhs) noexcept
  : extra_(std::move(rhs.extra_))
{

}

auto metadata::operator=(metadata&& rhs) noexcept -> metadata&
{
  extra_ = std::move(rhs.extra_);
  return *this;
}

metadata::~metadata() noexcept
{

}

auto metadata::meta_begin() -> meta_iterator
{
  return {this, 0, meta_proxy(0)};
}

auto metadata::meta_begin() const -> const_meta_iterator
{
  return {const_cast<metadata*>(this), 0, meta_proxy(0)};
}

auto metadata::meta_cbegin() const -> const_meta_iterator
{
  return {const_cast<metadata*>(this), 0, meta_proxy(0)};
}

auto metadata::meta_end() -> meta_iterator
{
  return {this, extra_.end()};
}

auto metadata::meta_end() const -> const_meta_iterator
{
  return {const_cast<metadata*>(this), const_cast<metadata*>(this)->extra_.end()};
}

auto metadata::meta_cend() const -> const_meta_iterator
{
  return {const_cast<metadata*>(this), const_cast<metadata*>(this)->extra_.end()};
}

auto metadata::meta_find(const std::string& name) -> meta_iterator
{
  for (auto i = extra_.begin(); i != extra_.end(); ++i)
    if (i->first == name)
      return {this, i};
  const proxy* ptr = meta_proxy(0);
  for (size_t i = 0; ptr != nullptr; ptr = meta_proxy(++i))
    if (ptr->name == name)
      return {this, i, ptr};
  return {this, extra_.end()};
}

auto metadata::meta_find(const std::string& name) const -> const_meta_iterator
{
  auto me = const_cast<metadata*>(this);
  for (auto i = me->extra_.begin(); i != me->extra_.end(); ++i)
    if (i->first == name)
      return {me, i};
  const proxy* ptr = meta_proxy(0);
  for (size_t i = 0; ptr != nullptr; ptr = meta_proxy(++i))
    if (ptr->name == name)
      return {me, i, ptr};
  return {me, me->extra_.end()};
}

auto metadata::meta_insert(const std::string& name) -> meta_iterator
{
  // ensure that it isn't already a proxy name
  const proxy* ptr = meta_proxy(0);
  for (size_t i = 0; ptr != nullptr; ptr = meta_proxy(++i))
  {
    if (ptr->name == name)
      return {this, i, ptr};
#if 0
      throw std::runtime_error{msg{} << "attempt to overwrite bound metadata: " << name};
#endif
  }

  // okay, insert it
  return meta_iterator{this, extra_.insert(std::make_pair(name, metavar{})).first};
}

auto metadata::meta_erase(const meta_iterator& iter) -> meta_iterator
{
  // ensure we aren't trying to erase a proxy entry
  if (iter.datum_.proxy_)
    throw std::runtime_error{
      msg{} << "attempt to erase bound metadata: " << iter.datum_.proxy_->name};

  return meta_iterator{this, extra_.erase(iter.datum_.extra_)};
}

auto metadata::meta_proxy(size_t i) const -> const proxy*
{
  return nullptr;
}

metadata::datum::datum(
      metadata* owner
    , size_t index
    , const proxy* prx
    , extra_store::iterator extra)
  : owner_(owner)
  , index_(index)
  , proxy_(prx)
  , extra_(extra)
{

}

auto metadata::datum::name() const -> const char*
{
  if (proxy_)
    return proxy_->name;
  return extra_->first.c_str();
}

auto metadata::datum::data_type() const -> type
{
  if (proxy_)
    return proxy_->impl->type();
  return extra_->second.which();
}

auto metadata::datum::bound() const -> bool
{
  return proxy_;
}

metadata::meta_iterator::meta_iterator(
      metadata* owner
    , size_t index
    , const proxy* prx)
  : datum_{owner, index, prx, owner->extra_.begin()}
{

}

metadata::meta_iterator::meta_iterator(metadata* owner, extra_store::iterator extra)
  : datum_{owner, 0, nullptr, extra}
{

}

// do _not_ compare index_! index is undefined for the end() iterator
auto metadata::meta_iterator::operator==(const meta_iterator& rhs) -> bool
{
  return 
       datum_.proxy_ == rhs.datum_.proxy_
    && datum_.extra_ == rhs.datum_.extra_
    && datum_.owner_ == rhs.datum_.owner_;
}

auto metadata::meta_iterator::operator!=(const meta_iterator& rhs) -> bool
{
  return 
       datum_.proxy_ != rhs.datum_.proxy_
    || datum_.extra_ != rhs.datum_.extra_
    || datum_.owner_ != rhs.datum_.owner_;
}

auto metadata::meta_iterator::operator++() -> meta_iterator&
{
  if (datum_.proxy_)
    datum_.proxy_ = datum_.owner_->meta_proxy(++datum_.index_);
  else
    ++datum_.extra_;
  return *this;
}

metadata::const_meta_iterator::const_meta_iterator(
      metadata* owner
    , size_t index
    , const proxy* prx)
  : datum_{owner, index, prx, owner->extra_.begin()}
{

}

metadata::const_meta_iterator::const_meta_iterator(metadata* owner, extra_store::iterator extra)
  : datum_{owner, 0, nullptr, extra}
{

}

// do _not_ compare index_! index is undefined for the end() iterator
auto metadata::const_meta_iterator::operator==(const const_meta_iterator& rhs) -> bool
{
  return 
       datum_.proxy_ == rhs.datum_.proxy_
    && datum_.extra_ == rhs.datum_.extra_
    && datum_.owner_ == rhs.datum_.owner_;
}

auto metadata::const_meta_iterator::operator!=(const const_meta_iterator& rhs) -> bool
{
  return 
       datum_.proxy_ != rhs.datum_.proxy_
    || datum_.extra_ != rhs.datum_.extra_
    || datum_.owner_ != rhs.datum_.owner_;
}

auto metadata::const_meta_iterator::operator++() -> const_meta_iterator&
{
  if (datum_.proxy_)
    datum_.proxy_ = datum_.owner_->meta_proxy(++datum_.index_);
  else
    ++datum_.extra_;
  return *this;
}

metadata::type_mismatch::type_mismatch(std::string name, type found, type requested)
  : std::logic_error(msg{} 
      << "metadata: type mismatch: \n"
      << "      field: " << name << '\n'
      << "      found: " << int(found) << '\n' // TODO - get name from type
      << "  requested: " << int(requested))
  , name_(std::move(name))
  , found_(found)
  , requested_(requested)
{ }

