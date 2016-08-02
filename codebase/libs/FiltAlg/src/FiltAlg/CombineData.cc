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
 * @file CombineData.cc
 */
#include <FiltAlg/CombineData.hh>
#include <FiltAlg/Filter.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
CombineData::CombineData(void)
{
  _is_input = false;
  _g = NULL;
  _is_conf_input = false;
  _c = NULL;
  _weight = 0.0;
}

//------------------------------------------------------------------
CombineData::CombineData(const string &name, const bool is_input, 
			 const double weight)
{
  _name = name;
  _is_input = is_input;
  _g = NULL;

  _conf_name = "";
  _is_conf_input = false;
  _c = NULL;

  _weight = weight;
}

//------------------------------------------------------------------
CombineData::CombineData(const string &name, const bool is_input, 
			 const string &conf_name, const bool conf_is_input, 
			 const double weight)
{
  _name = name;
  _is_input = is_input;
  _g = NULL;

  _conf_name = conf_name;
  _is_conf_input = conf_is_input;
  _c = NULL;

  _weight = weight;
}

//------------------------------------------------------------------
CombineData::~CombineData()
{
}

//------------------------------------------------------------------
bool CombineData::create_comb_data(const vector<Data> &input,
				   const vector<Data> &output)
{
  const Data *gin = Filter::set_data(_is_input, _name.c_str(), input, output);
  if (gin == NULL)
  {
    LOG(ERROR) << "input " << _name << " never found";
    return false;
  }
  _g = gin;

  if (_conf_name.empty())
  {
    _c = NULL;
  }
  else
  {
    gin = Filter::set_data(_is_conf_input, _conf_name.c_str(), input, output);
    if (gin == NULL)
    {
      LOG(ERROR) << "confidence input " << _name << " never found";
      return false;
    }
    _c = gin;
  }
  return true;
}

//------------------------------------------------------------------
const VlevelSlice *CombineData::matching_vlevel(const double vlevel,
						const double  tolerance) const
{
  if (_g == NULL)
  {
    LOG(ERROR) << "Getting matching vlevel when 3d grid missing";
    return NULL;
  }
  else
  {
    return _g->matching_vlevel(vlevel, tolerance);
  }
}

//------------------------------------------------------------------
const VlevelSlice *
CombineData::matching_conf_vlevel(const double vlevel,
				  const double  tolerance) const
{
  if (_c == NULL)
  {
    LOG(ERROR) << "getting matching vlevel confidence when 3d grid missing";
    return NULL;
  }
  else
  {
    return _c->matching_vlevel(vlevel, tolerance);
  }
}

//------------------------------------------------------------------
bool CombineData::type_equals(const Data::Data_t type) const
{
  bool stat = true;
  if (_g == NULL)
  {
    LOG(ERROR) << "checking type_equals when something missing";
    stat = false;
  }
  else
  {
    if (_g->get_type() != type)
    {
      LOG(ERROR) << "types dont match for combine " 
		 << Data::print_type(type) 
		 << " " 
		 << Data::print_type(_g->get_type());
      stat = false;
    }
  }

  if (_c != NULL)
  {
    if (_c->get_type() != type)
    {
      LOG(ERROR) << "conf types dont match for combine " 
		 << Data::print_type(type) 
		 << " " 
		 << Data::print_type(_c->get_type());
      stat = false;
    }
  }
  return stat;
}
