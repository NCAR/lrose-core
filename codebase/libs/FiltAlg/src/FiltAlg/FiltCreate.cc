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
 * @file FiltCreate.cc
 */
#include <FiltAlg/FiltCreate.hh>
#include <FiltAlg/Filter.hh>
#include <FiltAlg/Filt2d.hh>
#include <FiltAlg/Filt2dNoOverlap.hh>
#include <FiltAlg/FiltScalar.hh>
#include <FiltAlg/FiltClump.hh>
#include <FiltAlg/FiltCombine.hh>
#include <FiltAlg/FiltDB.hh>
#include <FiltAlg/FiltMask.hh>
#include <FiltAlg/FiltMaxTrue.hh>
#include <FiltAlg/FiltMedian.hh>
#include <FiltAlg/FiltMedianNoOverlap.hh>
#include <FiltAlg/FiltPassThrough.hh>
#include <FiltAlg/FiltRemap.hh>
#include <FiltAlg/FiltTrapRemap.hh>
#include <FiltAlg/FiltSRemap.hh>
#include <FiltAlg/FiltRescale.hh>
#include <FiltAlg/FiltReplace.hh>
#include <FiltAlg/FiltVertComb.hh>
#include <toolsa/LogStream.hh>

//------------------------------------------------------------------
Filter *FiltCreate::create(const FiltAlgParams::data_filter_t f,
			   const FiltAlgParms &P) const
{
  Filter *filt = NULL;
  switch (f.filter)
  {
  case FiltAlgParams::CLUMP:
    filt = new FiltClump(f, P);
    break;
  case FiltAlgParams::ELLIP:
  case FiltAlgParams::DILATE:
  case FiltAlgParams::SDEV:
  case FiltAlgParams::TEXTURE_X:
  case FiltAlgParams::TEXTURE_Y:
    filt = new Filt2d(f, P);
    break;
  case FiltAlgParams::MEDIAN:
    filt = new FiltMedian(f, P);
    break;
  case FiltAlgParams::SDEV_NO_OVERLAP:
    filt = new Filt2dNoOverlap(f, P);
    break;
  case FiltAlgParams::MEDIAN_NO_OVERLAP:
    filt = new FiltMedianNoOverlap(f, P);
    break;
  case FiltAlgParams::REMAP:
    filt = new FiltRemap(f, P);
    break;
  case FiltAlgParams::TRAPEZOID_REMAP:
    filt = new FiltTrapRemap(f, P);
    break;
  case FiltAlgParams::S_REMAP:
    filt = new FiltSRemap(f, P);
    break;
  case FiltAlgParams::RESCALE:
    filt = new FiltRescale(f, P);
    break;
  case FiltAlgParams::REPLACE:
    filt = new FiltReplace(f, P);
    break;
  case FiltAlgParams::MAX:
  case FiltAlgParams::AVERAGE:
  case FiltAlgParams::AVERAGE_ORIENTATION:
  case FiltAlgParams::PRODUCT:
  case FiltAlgParams::WEIGHTED_SUM:
  case FiltAlgParams::WEIGHTED_ORIENTATION_SUM:
  case FiltAlgParams::NORM_WEIGHTED_SUM:
  case FiltAlgParams::NORM_WEIGHTED_ORIENTATION_SUM:
    filt = new FiltCombine(f, P);
    break;
  case FiltAlgParams::MAX_TRUE:
    filt = new FiltMaxTrue(f, P);
    break;
  case FiltAlgParams::FULL_MEAN:
  case FiltAlgParams::FULL_SDEV:
  case FiltAlgParams::FULL_MEDIAN:
    filt = new FiltScalar(f, P);
    break;
  case FiltAlgParams::VERT_AVERAGE:
  case FiltAlgParams::VERT_MAX:
  case FiltAlgParams::VERT_PRODUCT:
    filt = new FiltVertComb(f, P);
    break;
  case FiltAlgParams::MASK:
    filt = new FiltMask(f, P);
    break;
  case FiltAlgParams::DB2LINEAR:
  case FiltAlgParams::LINEAR2DB:
    filt = new FiltDB(f, P);
    break;
  case FiltAlgParams::PASSTHROUGH:
    filt = new FiltPassThrough(f, P);
    break;
  case FiltAlgParams::APPFILTER:
    filt = appFiltCreate(f, P);
    break;
  default:
    LOG(ERROR) << "unknown filter " << f.filter;
    filt = NULL;
    break;
  }
  return filt;
}

/*----------------------------------------------------------------*/
Filter *FiltCreate::appFiltCreate(const FiltAlgParams::data_filter_t f,
				  const FiltAlgParms &P) const
{
  printf("ERROR, Should override appFiltCreate within app\n");
  return NULL;
}  
