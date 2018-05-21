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
 * @file FiltScalar.cc
 */
#include <FiltAlg/FiltScalar.hh>
#include <FiltAlg/Data2d.hh>
#include <FiltAlg/BasicInfo.hh>

//------------------------------------------------------------------
FiltScalar::FiltScalar(const FiltAlgParams::data_filter_t f, 
		       const FiltAlgParms &P) : Filter(f, P)
{
  _rLwr     = P._parm_Scalar[f.filter_index].rLwr;
  _rUpr     = P._parm_Scalar[f.filter_index].rUpr;
  _thetaLwr = P._parm_Scalar[f.filter_index].thetaLwr;
  _thetaUpr = P._parm_Scalar[f.filter_index].thetaUpr;

  if (!ok())
  {
    return;
  }
}

//------------------------------------------------------------------
FiltScalar::~FiltScalar()
{
}

//------------------------------------------------------------------
void FiltScalar::initialize_output(const Data &inp,
				   const FiltAlgParams::data_filter_t f,
				   Data &g)
{
  // output is always DATA2D for this filter
  g = Data(f.output_field, Data::DATA2D, f.write_output_field);
}

//------------------------------------------------------------------
void FiltScalar::filter_print(void) const
{
  LOG(DEBUG_VERBOSE) << "filtering";
}

//------------------------------------------------------------------
void FiltScalar::filter_print(const double vlevel) const
{
  LOG(DEBUG_VERBOSE) << "filtering vlevel=" << vlevel;
}

//------------------------------------------------------------------
bool FiltScalar::filter(const FiltInfoInput &inp, FiltInfoOutput &o) const
{
  if (!inp.hasVlevels())
  {
    LOG(ERROR) << "can't filter 1 data value";
    o.setBad();
    return false;
  }

  // input must be a grid
  if (!inp.getSlice()->is_grid())
  {
    LOG(ERROR) << "need grid input";
    o.setBad();
    return false;
  }

  GridAlgs g = GridAlgs::promote(*inp.getSlice());

  // convert upr and lwr bounds into indices in the grid
  // divide by dx (km) and dy (degrees) 
  // no wrapping

  int xLwr; /**< Lower x grid bound over which to calculate scalar stats. */
  int xUpr; /**< Upper x grid bound over which to calculate scalar stats. */
  int yLwr; /**< Lower y grid bound over which to calculate scalar stats. */
  int yUpr; /**< Upper y grid bound over which to calculate scalar stats. */


  xLwr = _rLwr/inp.getGridProj()._dx;
  xUpr = _rUpr/inp.getGridProj()._dx;
  yLwr = _thetaLwr/inp.getGridProj()._dy;
  yUpr = _thetaUpr/inp.getGridProj()._dy;

  double result;

  switch (_f.filter)
  {
  case FiltAlgParams::FULL_MEAN:
    result = g.localMeanXy(xLwr, xUpr, yLwr, yUpr);
    break;
  case FiltAlgParams::FULL_MEDIAN:
    result = g.localMedian(xLwr, xUpr, yLwr, yUpr);
    break;
  case FiltAlgParams::FULL_SDEV:
    result = g.localSdevXy(xLwr, xUpr, yLwr, yUpr);
    break;
  default:
    LOG(ERROR)<< "wrong filter";
    o.setBad();
    result = 0.0;
    return false;
  }

  // save out result
  o = FiltInfoOutput(result, NULL);
  return true;
}

//------------------------------------------------------------------
bool FiltScalar::create_inputs(const time_t &t, 
			       const vector<Data> &input,
			       const vector<Data> &output)
{
  return true;
}

//------------------------------------------------------------------
void FiltScalar::create_extra(FiltInfo &info) const
{
  return;
}

//------------------------------------------------------------------
bool FiltScalar::store_outputs(const Data &o, Info *info,
			       vector<FiltInfo> &extra,
			       vector<Data> &output)
{
  // store to output
  output.push_back(o);

  // get the Data2d from the output
  Data2d d, vlevels;
  o.construct_data2d(d, vlevels);

  // add to info
  info->add_data2d(d, vlevels);
  return true;
}

//------------------------------------------------------------------
void FiltScalar::set_info(Info **info) const
{
  // yes because we can write stuff to info (see above)
  if (*info == NULL)
  {
    BasicInfo *b = new BasicInfo();
    *info = (Info *)b;
  }
  // otherwise the base class is good enough
}

//------------------------------------------------------------------
void FiltScalar::set_input_info(Info **info) const
{
  return;
}

//------------------------------------------------------------------
void FiltScalar::vertical_level_change(void)
{
  // default is to do nothing
}

