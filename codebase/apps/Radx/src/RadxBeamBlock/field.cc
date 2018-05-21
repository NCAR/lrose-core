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

#include "field.h"

using namespace rainfields::ancilla;

static constexpr metadata::proxy_table meta_table = 
{
  metadata::make_proxy<field, std::string, &field::id, &field::set_id>("id")
};

field::field(std::string id)
  : id_(std::move(id))
{

}

// should be = default - but can't due to non-conforming std::string in gcc
auto field::operator=(field&& rhs) noexcept -> field&
{
  metadata::operator=(std::move(rhs));
  id_ = std::move(rhs.id_);
  return *this;
}

auto field::set_id(const std::string& val) -> void
{
  id_ = val;
}

auto field::meta_proxy(size_t i) const -> const proxy*
{
  if (i < std::extent<decltype(meta_table)>::value)
    return &meta_table[i];
  return nullptr;
}

field1::field1(std::string id, size_t size)
  : field(std::move(id))
  , array1<real>(size)
{ }

field1::field1(std::string id, const size_t dims[])
  : field(std::move(id))
  , array1<real>(dims)
{ }

field2::field2(std::string id, size_t y, size_t x)
  : field(std::move(id))
  , array2<real>(y, x)
{ }

field2::field2(std::string id, const size_t dims[])
  : field(std::move(id))
  , array2<real>(dims)
{ }

