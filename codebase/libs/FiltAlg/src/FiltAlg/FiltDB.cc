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
 * @file FiltDB.cc
 */
#include <FiltAlg/FiltDB.hh>

//------------------------------------------------------------------
FiltDB::FiltDB(const FiltAlgParams::data_filter_t f, const FiltAlgParms &P) :
  Filter(f, P)
{
}

//------------------------------------------------------------------
FiltDB::~FiltDB()
{
}

//------------------------------------------------------------------
void FiltDB::initialize_output(const Data &inp,
			       const FiltAlgParams::data_filter_t f, Data &g)
{
  g = Data(f.output_field, Data::GRID3D, f.write_output_field);
}

//------------------------------------------------------------------
void FiltDB::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltDB::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltDB::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!createGridAtVlevel(inp, o))
  {
    return false;
  }
  switch (_f.filter)
  {
  case FiltAlgParams::DB2LINEAR:
    o.db2linear();
    break;
  case FiltAlgParams::LINEAR2DB:
    o.linear2db();
    break;
  default:
    LOG(ERROR) << "wrong filter";
    o.setBad();
    return false;
  }
  return true;
}

//------------------------------------------------------------------
bool FiltDB::create_inputs(const time_t &t, 
			   const vector<Data> &input,
			   const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void FiltDB::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltDB::store_outputs(const Data &o, Info *info,
			   vector<FiltInfo> &extra,
			   vector<Data> &output)
{
  output.push_back(o);
  return true;
}

//------------------------------------------------------------------
void FiltDB::set_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltDB::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltDB::vertical_level_change(void)
{
  // default is to do nothing
}

