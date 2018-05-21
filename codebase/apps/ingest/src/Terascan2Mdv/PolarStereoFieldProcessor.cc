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
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/

// RCS info
//   $Author: cunning $
//   $Locker:  $
//   $Date: 2016/12/08 22:51:19 $
//   $Id: PolarStereoFieldProcessor.cc,v 1.3 2016/12/08 22:51:19 cunning Exp $
//   $Revision: 1.3 $
//   $State: Exp $
//
 
/**-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**/
/*********************************************************************
 * PolarStereoFieldProceSsor : Class the converts TeraScan data to a lat/lon
 *                        projection Mdv field.
 *
 * RAP, NCAR, Boulder CO
 *
 * October 2001
 *
 * Nancy Rehak
 *
 *********************************************************************/

#include <math.h>

#include "PolarStereoFieldProcessor.hh"

#include <euclid/euclid_macros.h>
#include <euclid/Pjg.hh>

using namespace std;


/*********************************************************************
 * Constructor
 */

PolarStereoFieldProcessor::PolarStereoFieldProcessor(SETP input_dataset) :
  FieldProcessor(input_dataset)
{
  // Do nothing
}


/*********************************************************************
 * Destructor
 */

PolarStereoFieldProcessor::~PolarStereoFieldProcessor()
{
  // Do nothing
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcIndex() - Calculate the MDV field index for the given satellite
 *                field location.
 */

int PolarStereoFieldProcessor::_calcIndex(const int x_index, const int y_index)
{
  return x_index * _numSamples + y_index;
}


/*********************************************************************
 * _initLocal() - Do any initialization needed locally by the derived
 *                class.  This method is called at the end of the
 *                FieldProcessor::_init() method.
 */

bool PolarStereoFieldProcessor::_initLocal(ETXFORM mxfm)
{

  int etgetproj P_((ETXST xfm));

  double adjacent_lat, adjacent_lon;  

  double upperLeftLat, upperLeftLon;
  double lowerRightLat, lowerRightLon;
  double upperRightLat, upperRightLon;
  double etdx, etdy;
  double etcx, etcy;
  double etllx, etlly;
  double etulx, etuly;
  double etlrx, etlry;
  double eturx, etury;

  etxll(mxfm, _numLines, 1, &_lowerLeftLat, &_lowerLeftLon);
  llproj(mxfm, _lowerLeftLat, _lowerLeftLon, &etllx, &etlly);

  etxll(mxfm, (_numLines - 1), 1, &adjacent_lat, &adjacent_lon);
  llproj(mxfm, adjacent_lat, adjacent_lon, &etdx, &etdy);
  _deltaX = (etllx - etdx);
  //_deltaX = 1.43;


  etxll(mxfm, _numLines, 2, &adjacent_lat, &adjacent_lon);
  llproj(mxfm, adjacent_lat, adjacent_lon, &etdx, &etdy);
  _deltaY = (etlly - etdy);
  //_deltaY = 1.42;


  Pjg polar_pjg;
  polar_pjg.initPolarStereo(_centerLon, PjgTypes::POLE_NORTH, 1.0, 
			    _numSamples, _numLines);
 
  double loc_minx, loc_miny;

  polar_pjg.latlon2xy(_lowerLeftLat, _lowerLeftLon, loc_minx, loc_miny);

  polar_pjg.setGridMins(loc_minx, loc_miny, 1.0);

  _minX = polar_pjg. getMinx();
  _minY = polar_pjg. getMiny();

   cout << "dx = " << _deltaX << "  dy = " << _deltaY << endl;
   cout << "dxs = " << (_deltaX*_mapScaleFactor) << "  dys = " << (_deltaY*_mapScaleFactor) << endl;
   cout << "minx = " << _minX << "  miny = " << _minY << endl;
 
  return true;
}


/*********************************************************************
 * _setProjectionInfo() - Set the projection information in the given
 *                        field header.
 */

void PolarStereoFieldProcessor::_setProjectionInfo(Mdvx::field_header_t &fld_hdr)
{
  fld_hdr.proj_type = Mdvx::PROJ_POLAR_STEREO;
  
  fld_hdr.grid_dy = _deltaX * _mapScaleFactor;
  fld_hdr.grid_dx = _deltaY * _mapScaleFactor;
  fld_hdr.grid_minx = _minX;
  fld_hdr.grid_miny = _minY;
  fld_hdr.proj_param[0] =  _centerLon;
  fld_hdr.proj_param[1] =  0.0;
  fld_hdr.proj_param[2] =  1.0;
}
