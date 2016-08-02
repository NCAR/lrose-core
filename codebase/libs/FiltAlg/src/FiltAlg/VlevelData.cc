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
 * @file VlevelData.cc
 */
#include <cmath>
#include <FiltAlg/VlevelData.hh>

//------------------------------------------------------------------
VlevelData::VlevelData(const string &name, const bool disabled, const bool grid)
{
  _name = name;
  _disabled = disabled;
  _gridded = grid;
}

//------------------------------------------------------------------
VlevelData::~VlevelData()
{
}

//------------------------------------------------------------------
void VlevelData::print_vlevel_data(void) const
{
  for (int i=0; i<static_cast<int>(_g.size()); ++i)
  {
    _g[i].print_slice();
  }
}

//------------------------------------------------------------------
int VlevelData::num_vlevel(void) const
{
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return 0;
  }
  else
  {
    return static_cast<int>(_g.size());
  }
}

//------------------------------------------------------------------
vector<double> VlevelData::extract_vlevel(void) const
{
  vector<double> ret;
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return ret;
  }
  for (int i=0; i<static_cast<int>(_g.size()); ++i)
  {
    ret.push_back(_g[i].get_vlevel());
  }
  return ret;
}

//------------------------------------------------------------------
const VlevelSlice *VlevelData::matching_vlevel(const double vlevel,
					       const double tolerance) const
{
  for (int i=0; i<static_cast<int>(_g.size()); ++i)
  {
    if (fabs(vlevel - _g[i].get_vlevel()) <= tolerance)
    {
      return &_g[i];
    }
  }
  return NULL;
}

//------------------------------------------------------------------
fl32 *VlevelData::volume(fl32 &bad, fl32 &missing) const
{
  fl32 *ret = NULL;
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return ret;
  }
  if (!_gridded)
  {
    LOG(ERROR) << _name << " not a 3d grid";
    return ret;
  }

  int nz = static_cast<int>(_g.size());
  if (nz == 0)
  {
    return ret;
  }
  int nxy = _g[0].getNdata();
  ret = new fl32[nxy*nz];
  for (int i=0; i<nz; ++i)
  {
    for (int j=0; j<nxy; ++j)
    {
      ret[i*nxy + j] = static_cast<fl32>(_g[i].getValue(j));
    }
    // memcpy(&ret[i*nxy], _g[i].get_data(), nxy*sizeof(fl32));
    bad = _g[i].getMissing();
    missing = _g[i].getMissing();
  }
  return ret;
}

//------------------------------------------------------------------
bool VlevelData::dim_bad(const int nx, const int ny, const int nz,
			 bool print) const
{
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return false;
  }
  if (!_gridded)
  {
    LOG(ERROR) << _name << " not a 3d grid";
    return false;
  }

  if (nz != static_cast<int>(_g.size()))
  {
    if (print)
    {
      LOG(ERROR) << "nz=" << nz << ", vlevels size=" << _g.size();
    }
    return true;
  }
  if (nz == 0)
  {
    return false;
  }

  if (nx != _g[0].getNx() || ny != _g[0].getNy())
  {
    if (print)
    {
      LOG(ERROR) << "nx,ny=" << nx << "," << ny <<"  vlevel dim="
		 << _g[0].getNx() << "," <<  _g[0].getNy();
    }
    return true;
  }
  return false;
}

//------------------------------------------------------------------
void VlevelData::clear(void)
{
  _g.clear();
}

//------------------------------------------------------------------
bool VlevelData::add(const Grid2d &g, const double vlevel, 
		     const int vlevel_index, const GridProj &gp)
{
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return false;
  }
  if (!_gridded)
  {
    LOG(ERROR) << _name << " not a 3d grid";
    return false;
  }

  if (!_g.empty())
  {
    if (!_g[0].dimensionsEqual(g))
    {
      LOG(ERROR) << _name << " dims unequal";
      return false;
    }
  }
  VlevelSlice s(_name, g, vlevel, vlevel_index, gp);
  _g.push_back(s);
  return true;
}

//------------------------------------------------------------------
bool VlevelData::add(const double vlevel, const int vlevel_index,
		     const double value)
{
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return false;
  }
  if (_gridded)
  {
    LOG(ERROR) << _name << " gridded want non gridded";
    return false;
  }

  VlevelSlice s(_name, value, vlevel, vlevel_index);
  _g.push_back(s);
  return true;
}

//------------------------------------------------------------------
bool VlevelData::store_2d_info(Data2d &d, Info &info) const
{
  Data2d vl(Info::data2d_vlevel_name());
  for (int i=0; i<static_cast<int>(_g.size()); ++i)
  {
    double v;
    if (_g[i].get_1d_value(v))
    {
      d.add(v);
    }
    else
    {
      LOG(ERROR) << "getting 1d value";
      return false;
    }
    v = _g[i].get_vlevel();
    vl.add(v);
  }
  info.add_data2d(d, vl);
  return true;
}

//------------------------------------------------------------------
bool VlevelData::construct_data2d(Data2d &d, Data2d &vlevels) const
{
  if (_disabled)
  {
    LOG(ERROR) << _name << " disabled";
    return false;
  }
  if (_gridded)
  {
    LOG(ERROR) << _name << " gridded want non gridded";
    return false;
  }

  string vname = Info::data2d_vlevel_name();
  d = Data2d(_name);
  vlevels = Data2d(vname);
  for (int i=0; i<static_cast<int>(_g.size()); ++i)
  {
    double v;
    if (_g[i].get_1d_value(v))
    {
      d.add(v);
      v = _g[i].get_vlevel();
      vlevels.add(v);
    }
    else
    {
      LOG(ERROR) << "constructing from Vlevel data";
      return false;
    }
  }
  return true;
}

