// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
// ** Copyright UCAR (c) 1990 - 2016                                         
// ** University Corporation for Atmospheric Research (UCAR)                 
// ** National Center for Atmospheric Research (NCAR)                        
// ** Boulder, Colorado, USA                                                 
// ** BSD licence applies - redistribution and use in source and binary      
// ** forms, with or without modification, are permitted provided that       
// ** the following conditions are met:                                      
// ** 1) If the software is modified to produce derivative works,            
// ** such modified software should be clearly marked, so as not             
// ** to confuse it with the version available from UCAR.                    
// ** 2) Redistributions of source code must retain the above copyright      
// ** notice, this list of conditions and the following disclaimer.          
// ** 3) Redistributions in binary form must reproduce the above copyright   
// ** notice, this list of conditions and the following disclaimer in the    
// ** documentation and/or other materials provided with the distribution.   
// ** 4) Neither the name of UCAR nor the names of its contributors,         
// ** if any, may be used to endorse or promote products derived from        
// ** this software without specific prior written permission.               
// ** DISCLAIMER: THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS  
// ** OR IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED      
// ** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.    
// *=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=* 
/**
 * @file VlevelSlice.cc
 */
#include <FiltAlg/VlevelSlice.hh>
#include <euclid/Grid2d.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
VlevelSlice::VlevelSlice() : GridAlgs(), Data1d()
{
  _name = "unknown";
  _vlevel = 0;
  _vlevel_index = -1;
}

//------------------------------------------------------------------
VlevelSlice::VlevelSlice(const string &name, const Grid2d &g,
			 const double vlevel, const int vlevel_index,
			 const GridProj &gp) :  GridAlgs(g), Data1d()
{
  _name = name;
  _vlevel = vlevel;
  _vlevel_index = vlevel_index;
  _grid_proj = gp;
  _is_data = false;
}

//------------------------------------------------------------------
VlevelSlice::VlevelSlice(const string &name, const double value,
			 const double vlevel, const int vlevel_index) :
  GridAlgs(), Data1d(name, value)
{
  _name = name;
  _vlevel = vlevel;
  _vlevel_index = vlevel_index;
  _is_data = true;
}

//------------------------------------------------------------------
VlevelSlice::~VlevelSlice()
{
}

//------------------------------------------------------------------
void VlevelSlice::print_slice(void) const
{
  printf("VlevelSlice: vlevel[%d]:%lf ", _vlevel_index, _vlevel);
  if (_is_data)
  {
    print_1d();
  }
  else
  {
    _grid_proj.print();
    Grid2d::print();
  }
}

//------------------------------------------------------------------
void VlevelSlice::print_vlevel(void) const
{
  printf("Vlevel[%d]:%lf\n", _vlevel_index, _vlevel);
}

//------------------------------------------------------------------
void VlevelSlice::set_name(const string &name)
{
  _name = name;
  if (_is_data)
  {
    set_1d_name(name);
  }
}

//------------------------------------------------------------------
bool VlevelSlice::max_slice(const VlevelSlice &v)
{
  if (_is_data != v._is_data)
  {
    LOG(ERROR) << "unequal types";
    return false;
  }
  if (_is_data)
  {
    double input_value, loc_value;
    if (!v.get_1d_value(input_value))
    {
      LOG(ERROR) << "weird error";
      return false;
    }
    if (!get_1d_value(loc_value))
    {
      LOG(ERROR) << "weird error2";
      return false;
    }
    if (input_value > loc_value)
    {
      return set_1d_value(input_value);
    }
    else
    {
      return true;
    }
  }
  else
  {
    // call the grid method
    GridAlgs::max(v);
    return true;
  }
}

//------------------------------------------------------------------
bool VlevelSlice::product_slice(const VlevelSlice &v)
{
  if (_is_data != v._is_data)
  {
    LOG(ERROR) << "unequal types";
    return false;
  }
  if (_is_data)
  {
    double input_value, loc_value;
    if (!v.get_1d_value(input_value))
    {
      LOG(ERROR) << "weird error";
      return false;
    }
    if (!get_1d_value(loc_value))
    {
      LOG(ERROR) << "weird error 2";
      return false;
    }
    loc_value *= input_value;
    return set_1d_value(loc_value);
  }
  else
  {
    // call the grid method
    GridAlgs::multiply(v);
    return true;
  }
}

//------------------------------------------------------------------
bool VlevelSlice::divide_slice(const VlevelSlice &v)
{
  if (_is_data != v._is_data)
  {
    LOG(ERROR) << "unequal types";
    return false;
  }
  if (_is_data)
  {
    double input_value, loc_value;
    if (!v.get_1d_value(input_value))
    {
      LOG(ERROR) << "weird error";
      return false;
    }
    if (!get_1d_value(loc_value))
    {
      LOG(ERROR) << "weird error 2";
      return false;
    }
    if (input_value != 0.0)
    {
      loc_value /= input_value;
    }
    else
    {
      LOG(ERROR) << "Cannot divide by zero";
    }
    return set_1d_value(loc_value);
  }
  else
  {
    // call the grid method
    GridAlgs::divide(v);
    return true;
  }
}

//------------------------------------------------------------------
bool VlevelSlice::add_slice(const VlevelSlice &v)
{
  if (_is_data != v._is_data)
  {
    LOG(ERROR) << "unequal types";
    return false;
  }
  if (_is_data)
  {
    double value;
    if (get_1d_value(value))
    {
      Data1d::inc_1d_value(value);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    GridAlgs::add(v);
    return true;
  }
}

//------------------------------------------------------------------
void VlevelSlice::multiply_slice(const double value)
{
  if (_is_data)
  {
    Data1d::prod_1d_value(value);
  }
  else
  {
    GridAlgs::multiply(value);
  }
}

//------------------------------------------------------------------
void VlevelSlice::add_value_to_slice(const double value)
{
  if (_is_data)
  {
    Data1d::inc_1d_value(value);
  }
  else
  {
    GridAlgs::add(value);
  }
}

//------------------------------------------------------------------
void VlevelSlice::set_slice(const double value)
{
  if (_is_data)
  {
    Data1d::set_1d_value(value);
  }
  else
  {
    Grid2d::setAllToValue(value);
  }
}

//------------------------------------------------------------------
void VlevelSlice::set_slice_empty(void)
{
  if (_is_data)
  {
    Data1d::set_1d_value(0.0);
  }
  else
  {
    Grid2d::setAllMissing();
  }
}

//------------------------------------------------------------------
void VlevelSlice::init_average(VlevelSlice &counts)
{
  // make it same type
  counts = *this;

  // init to 0 
  counts.set_slice(0.0);

  // create a temp empty slice
  VlevelSlice tmp = *this;
  tmp.set_slice_empty();

  // accum into that
  tmp.accum_average(*this, counts);

  // replace local with accum
  *this = tmp;
}

//------------------------------------------------------------------
bool VlevelSlice::accum_average(const VlevelSlice &data, VlevelSlice &counts)
{
  if (_is_data != data._is_data || _is_data != counts._is_data)
  {
    LOG(ERROR) << "unequal types";
    return false;
  }

  if (_is_data)
  {
    if (!add_1d_value(data))
    {
      LOG(ERROR) << "weird error 0";
      return false;
    }
    if (!counts.inc_1d_value(1.0))
    {
      LOG(ERROR) << "weird error 1";
      return false;
    }
  }
  else
  {
    add(data, counts);
  }
  return true;
}

//------------------------------------------------------------------
void VlevelSlice::finish_average(const VlevelSlice &counts)
{
  if (_is_data)
  {
    double v;
    double count;
    if (get_1d_value(v) && counts.get_1d_value(count))
    {
      if (count > 0)
      {
	v = v/count;
      }
      else
      {
	v = 0.0;
      }
    }
    set_1d_value(v);
  }
  else
  {
    GridAlgs::divide(counts);
  }
}

